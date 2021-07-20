/**********************************************************************

Audacium: A Digital Audio Editor

Registry.cpp

Paul Licameli split from Menus.cpp

**********************************************************************/

#include "Registry.h"

#include <unordered_set>

#include <wx/log.h>

#include "widgets/AudacityMessageBox.h"

namespace {

struct ItemOrdering;

using namespace Registry;
struct CollectedItems
{
   struct Item{
      // Predefined, or merged from registry already:
      BaseItem *visitNow;
      // Corresponding item from the registry, its sub-items to be merged:
      GroupItem *mergeLater;
      // Ordering hint for the merged item:
      OrderingHint hint;
   };
   std::vector< Item > items;
   std::vector< BaseItemSharedPtr > &computedItems;

   // A linear search.  Smarter search may not be worth the effort.
   using Iterator = decltype( items )::iterator;
   auto Find( const Identifier &name ) -> Iterator
   {
      auto end = items.end();
      return name.empty()
         ? end
         : std::find_if( items.begin(), end,
            [&]( const Item& item ){
               return name == item.visitNow->name; } );
   }

   auto InsertNewItemUsingPreferences(
      ItemOrdering &itemOrdering, BaseItem *pItem ) -> bool;

   auto InsertNewItemUsingHint(
      BaseItem *pItem, const OrderingHint &hint, size_t endItemsCount,
      bool force )
         -> bool;

   auto MergeLater( Item &found, const Identifier &name ) -> GroupItem *;

   auto SubordinateSingleItem( Item &found, BaseItem *pItem ) -> void;

   auto SubordinateMultipleItems( Item &found, GroupItem *pItems ) -> void;

   auto MergeWithExistingItem(
      Visitor &visitor, ItemOrdering &itemOrdering, BaseItem *pItem ) -> bool;

   using NewItem = std::pair< BaseItem*, OrderingHint >;
   using NewItems = std::vector< NewItem >;

   auto MergeLikeNamedItems(
      Visitor &visitor, ItemOrdering &itemOrdering,
      NewItems::const_iterator left, NewItems::const_iterator right,
      int iPass, size_t endItemsCount, bool force )
         -> bool;

   auto MergeItemsAscendingNamesPass(
      Visitor &visitor, ItemOrdering &itemOrdering,
      NewItems &newItems, int iPass, size_t endItemsCount, bool force )
         -> void;

   auto MergeItemsDescendingNamesPass(
      Visitor &visitor, ItemOrdering &itemOrdering,
      NewItems &newItems, int iPass, size_t endItemsCount, bool force )
         -> void;

   auto MergeItems(
      Visitor &visitor, ItemOrdering &itemOrdering,
      const BaseItemPtrs &toMerge, const OrderingHint &hint ) -> void;
};

// When a computed or shared item, or nameless grouping, specifies a hint and
// the subordinate does not, propagate the hint.
const OrderingHint &ChooseHint(BaseItem *delegate, const OrderingHint &hint)
{
   return !delegate || delegate->orderingHint.type == OrderingHint::Unspecified
      ? hint
      : delegate->orderingHint;
}

// "Collection" of items is the first pass of visitation, and resolves
// delegation and delayed computation and splices transparent group nodes.
// This first pass is done at each group, starting with a top-level group.
// This pass does not descend to the leaves.  Rather, the visitation passes
// alternate as the entire tree is recursively visited.

// forward declaration for mutually recursive functions
void CollectItem( Registry::Visitor &visitor,
   CollectedItems &collection, BaseItem *Item, const OrderingHint &hint );
void CollectItems( Registry::Visitor &visitor,
   CollectedItems &collection, const BaseItemPtrs &items,
   const OrderingHint &hint )
{
   for ( auto &item : items )
      CollectItem( visitor, collection, item.get(),
         ChooseHint( item.get(), hint ) );
}
void CollectItem( Registry::Visitor &visitor,
   CollectedItems &collection, BaseItem *pItem, const OrderingHint &hint )
{
   if (!pItem)
      return;

   using namespace Registry;
   if (const auto pShared =
       dynamic_cast<SharedItem*>( pItem )) {
      auto delegate = pShared->ptr.get();
      if ( delegate )
         // recursion
         CollectItem( visitor, collection, delegate,
            ChooseHint( delegate, pShared->orderingHint ) );
   }
   else
   if (const auto pComputed =
       dynamic_cast<ComputedItem*>( pItem )) {
      auto result = pComputed->factory( visitor );
      if (result) {
         // Guarantee long enough lifetime of the result
         collection.computedItems.push_back( result );
         // recursion
         CollectItem( visitor, collection, result.get(),
            ChooseHint( result.get(), pComputed->orderingHint ) );
      }
   }
   else
   if (auto pGroup = dynamic_cast<GroupItem*>(pItem)) {
      if (pGroup->Transparent() && pItem->name.empty())
         // nameless grouping item is transparent to path calculations
         // collect group members now
         // recursion
         CollectItems(
            visitor, collection, pGroup->items, ChooseHint( pGroup, hint ) );
      else
         // all other group items
         // defer collection of members until collecting at next lower level
         collection.items.push_back( {pItem, nullptr, hint} );
   }
   else {
      wxASSERT( dynamic_cast<SingleItem*>(pItem) );
      // common to all single items
      collection.items.push_back( {pItem, nullptr, hint} );
   }
}

using Path = std::vector< Identifier >;

   std::unordered_set< wxString > sBadPaths;
   void BadPath(
     const TranslatableString &format, const wxString &key, const Identifier &name )
   {
     // Warn, but not more than once in a session for each bad path
     auto badPath = key + '/' + name.GET();
     if ( sBadPaths.insert( badPath ).second ) {
        auto msg = TranslatableString{ format }.Format( badPath );
        // debug message
        wxLogDebug( msg.Translation() );
#ifdef IS_ALPHA
        // user-visible message
        AudacityMessageBox( msg );
#endif
     }
   }

   void ReportGroupGroupCollision( const wxString &key, const Identifier &name )
   {
      BadPath(
XO("Plug-in group at %s was merged with a previously defined group"),
         key, name);
   }

   void ReportItemItemCollision( const wxString &key, const Identifier &name )
   {
      BadPath(
XO("Plug-in item at %s conflicts with a previously defined item and was discarded"),
         key, name);
   }

   void ReportConflictingPlacements( const wxString &key, const Identifier &name )
   {
      BadPath(
XO("Plug-in items at %s specify conflicting placements"),
         key, name);
   }

struct ItemOrdering {
   wxString key;

   ItemOrdering( const Path &path )
   {
      // The set of path names determines only an unordered tree.
      // We want an ordering of the tree that is stable across runs.
      // The last used ordering for this node can be found in preferences at this
      // key:
      wxArrayString strings;
      for (const auto &id : path)
         strings.push_back( id.GET() );
      key = '/' + ::wxJoin( strings, '/', '\0' );
   }

   // Retrieve the old ordering on demand, if needed to merge something.
   bool gotOrdering = false;
   wxString strValue;
   wxArrayString ordering;

   auto Get() -> wxArrayString & {
      if ( !gotOrdering ) {
         gPrefs->Read(key, &strValue);
         ordering = ::wxSplit( strValue, ',' );
         gotOrdering = true;
      }
      return ordering;
   };
};

// For each group node, this is called only in the first pass of merging of
// items.  It might fail to place an item in the first visitation of a
// registry, but then succeed in later visitations in the same or later
// runs of the program, because of persistent side-effects on the
// preferences done at the very end of the visitation.
auto CollectedItems::InsertNewItemUsingPreferences(
   ItemOrdering &itemOrdering, BaseItem *pItem )
   -> bool
{
   // Note that if more than one plug-in registers items under the same
   // node, then it is not specified which plug-in is handled first,
   // the first time registration happens.  It might happen that you
   // add a plug-in, run the program, then add another, then run again;
   // registration order determined by those actions might not
   // correspond to the order of re-loading of modules in later
   // sessions.  But whatever ordering is chosen the first time some
   // plug-in is seen -- that ordering gets remembered in preferences.

   if ( !pItem->name.empty() ) {
      // Check saved ordering first, and rebuild that as well as is possible
      auto &ordering = itemOrdering.Get();
      auto begin2 = ordering.begin(), end2 = ordering.end(),
         found2 = std::find( begin2, end2, pItem->name );
      if ( found2 != end2 ) {
         auto insertPoint = items.end();
         // Find the next name in the saved ordering that is known already
         // in the collection.
         while ( ++found2 != end2 ) {
            auto known = Find( *found2 );
            if ( known != insertPoint ) {
               insertPoint = known;
               break;
            }
         }
         items.insert( insertPoint, {pItem, nullptr,
            // Hints no longer matter:
            {}} );
         return true;
      }
   }

   return false;
}

// For each group node, this may be called in the second and later passes
// of merging of items
auto CollectedItems::InsertNewItemUsingHint(
   BaseItem *pItem, const OrderingHint &hint, size_t endItemsCount,
   bool force ) -> bool
{
   auto begin = items.begin(), end = items.end(),
      insertPoint = end - endItemsCount;

   // pItem should have a name; if not, ignore the hint, and put it at the
   // default place, but only if in the final pass.
   if ( pItem->name.empty() ) {
      if ( !force )
         return false;
   }
   else {
      switch ( hint.type ) {
         case OrderingHint::Before:
         case OrderingHint::After: {
            // Default to the end if the name is not found.
            auto found = Find( hint.name );
            if ( found == end ) {
               if ( !force )
                  return false;
               else
                  insertPoint = found;
            }
            else {
               insertPoint = found;
               if ( hint.type == OrderingHint::After )
                  ++insertPoint;
            }
            break;
         }
         case OrderingHint::Begin:
            insertPoint = begin;
            break;
         case OrderingHint::End:
            insertPoint = end;
            break;
         case OrderingHint::Unspecified:
         default:
            if ( !force )
               return false;
            break;
      }
   }

   // Insert the item; the hint has been used and no longer matters
   items.insert( insertPoint, {pItem, nullptr,
      // Hints no longer matter:
      {}} );
   return true;
}

auto CollectedItems::MergeLater( Item &found, const Identifier &name )
   -> GroupItem *
{
   auto subGroup = found.mergeLater;
   if ( !subGroup ) {
      auto newGroup = std::make_shared<TransparentGroupItem<>>( name );
      computedItems.push_back( newGroup );
      subGroup = found.mergeLater = newGroup.get();
   }
   return subGroup;
}

auto CollectedItems::SubordinateSingleItem( Item &found, BaseItem *pItem )
   -> void
{
   MergeLater( found, pItem->name )->items.push_back(
      std::make_unique<SharedItem>(
         // shared pointer with vacuous deleter
         std::shared_ptr<BaseItem>( pItem, [](void*){} ) ) );
}

auto CollectedItems::SubordinateMultipleItems( Item &found, GroupItem *pItems )
   -> void
{
   auto subGroup = MergeLater( found, pItems->name );
   for ( const auto &pItem : pItems->items )
      subGroup->items.push_back( std::make_unique<SharedItem>(
         // shared pointer with vacuous deleter
         std::shared_ptr<BaseItem>( pItem.get(), [](void*){} ) ) );
}

auto CollectedItems::MergeWithExistingItem(
   Visitor &visitor, ItemOrdering &itemOrdering, BaseItem *pItem ) -> bool
{
   // Assume no null pointers remain after CollectItems:
   const auto &name = pItem->name;
   const auto found = Find( name );
   if (found != items.end()) {
      // Collision of names between collection and registry!
      // There are 2 * 2 = 4 cases, as each of the two are group items or
      // not.
      auto pCollectionGroup = dynamic_cast< GroupItem * >( found->visitNow );
      auto pRegistryGroup = dynamic_cast< GroupItem * >( pItem );
      if (pCollectionGroup) {
         if (pRegistryGroup) {
            // This is the expected case of collision.
            // Subordinate items from one of the groups will be merged in
            // another call to MergeItems at a lower level of path.
            // Note, however, that at most one of the two should be other
            // than a plain grouping item; if not, we must lose the extra
            // information carried by one of them.
            bool pCollectionGrouping = pCollectionGroup->Transparent();
            auto pRegistryGrouping = pRegistryGroup->Transparent();
            if ( !(pCollectionGrouping || pRegistryGrouping) )
               ReportGroupGroupCollision( itemOrdering.key, name );

            if ( pCollectionGrouping && !pRegistryGrouping ) {
               // Swap their roles
               found->visitNow = pRegistryGroup;
               SubordinateMultipleItems( *found, pCollectionGroup );
            }
            else
               SubordinateMultipleItems( *found, pRegistryGroup );
         }
         else {
            // Registered non-group item collides with a previously defined
            // group.
            // Resolve this by subordinating the non-group item below
            // that group.
            SubordinateSingleItem( *found, pItem );
         }
      }
      else {
         if (pRegistryGroup) {
            // Subordinate the previously merged single item below the
            // newly merged group.
            // In case the name occurred in two different static registries,
            // the final merge is the same, no matter which is treated first.
            auto demoted = found->visitNow;
            found->visitNow = pRegistryGroup;
            SubordinateSingleItem( *found, demoted );
         }
         else
            // Collision of non-group items is the worst case!
            // The later-registered item is lost.
            // Which one you lose might be unpredictable when both originate
            // from static registries.
            ReportItemItemCollision( itemOrdering.key, name );
      }
      return true;
   }
   else
      // A name is registered that is not known in the collection.
      return false;
}

auto CollectedItems::MergeLikeNamedItems(
   Visitor &visitor, ItemOrdering &itemOrdering,
   NewItems::const_iterator left, NewItems::const_iterator right,
   const int iPass, size_t endItemsCount, bool force )
   -> bool
{
   // Try to place the first item of the range.
   // If such an item is a group, then we always retain the kind of
   // grouping that was registered.  (Which doesn't always happen when
   // there is name collision in MergeWithExistingItem.)
   auto iter = left;
   auto &item = *iter;
   auto pItem = item.first;
   const auto &hint = item.second;
   bool success = false;
   if ( iPass == -1 )
      // A first pass consults preferences.
      success = InsertNewItemUsingPreferences( itemOrdering, pItem );
   else if ( iPass == hint.type ) {
      // Later passes for choosing placements.
      // Maybe it fails in this pass, because a placement refers to some
      // other name that has not yet been placed.
      success =
         InsertNewItemUsingHint( pItem, hint, endItemsCount, force );
      wxASSERT( !force || success );
   }

   if ( success ) {
      // Resolve collisions among remaining like-named items.
      ++iter;
      if ( iter != right && iPass != 0 &&
          iter->second.type != OrderingHint::Unspecified &&
          !( iter->second == hint ) ) {
         // A diagnostic message sometimes
         ReportConflictingPlacements( itemOrdering.key, pItem->name );
      }
      while ( iter != right )
         // Re-invoke MergeWithExistingItem for this item, which is known
         // to have a name collision, so ignore the return value.
         MergeWithExistingItem( visitor, itemOrdering, iter++ -> first );
   }

   return success;
}

inline bool MajorComp(
   const CollectedItems::NewItem &a, const CollectedItems::NewItem &b) {
   // Descending sort!
   return a.first->name > b.first->name;
};
inline bool MinorComp(
   const CollectedItems::NewItem &a, const CollectedItems::NewItem &b){
   // Sort by hint type.
   // This sorts items with unspecified hints last.
   return a.second < b.second;
};
inline bool Comp(
   const CollectedItems::NewItem &a, const CollectedItems::NewItem &b){
   if ( MajorComp( a, b ) )
      return true;
   if ( MajorComp( b, a ) )
      return false;
   return MinorComp( a, b );
};

auto CollectedItems::MergeItemsAscendingNamesPass(
  Visitor &visitor, ItemOrdering &itemOrdering, NewItems &newItems,
  const int iPass, size_t endItemsCount, bool force ) -> void
{
   // Inner loop over ranges of like-named items.
   auto rright = newItems.rbegin();
   auto rend = newItems.rend();
   while ( rright != rend ) {
      // Find the range
      using namespace std::placeholders;
      auto rleft = std::find_if(
         rright + 1, rend, std::bind( MajorComp, _1, *rright ) );

      bool success = MergeLikeNamedItems(
         visitor, itemOrdering, rleft.base(), rright.base(), iPass,
         endItemsCount, force );

      if ( success ) {
         auto diff = rend - rleft;
         newItems.erase( rleft.base(), rright.base() );
         rend = newItems.rend();
         rleft = rend - diff;
      }
      rright = rleft;
   }
}

auto CollectedItems::MergeItemsDescendingNamesPass(
  Visitor &visitor, ItemOrdering &itemOrdering, NewItems &newItems,
  const int iPass, size_t endItemsCount, bool force ) -> void
{
   // Inner loop over ranges of like-named items.
   auto left = newItems.begin();
   while ( left != newItems.end() ) {
      // Find the range
      using namespace std::placeholders;
      auto right = std::find_if(
         left + 1, newItems.end(), std::bind( MajorComp, *left, _1 ) );

      bool success = MergeLikeNamedItems(
         visitor, itemOrdering, left, right, iPass,
         endItemsCount, force );

      if ( success )
         left = newItems.erase( left, right );
      else
         left = right;
   }
};

auto CollectedItems::MergeItems(
  Visitor &visitor, ItemOrdering &itemOrdering,
  const BaseItemPtrs &toMerge, const OrderingHint &hint ) -> void
{
   // First do expansion of nameless groupings, and caching of computed
   // items, just as for the previously collected items.
   CollectedItems newCollection{ {}, computedItems };
   CollectItems( visitor, newCollection, toMerge, hint );

   // Try to merge each, resolving name collisions with items already in the
   // tree, and collecting those with names that don't collide.
   NewItems newItems;
   for ( const auto &item : newCollection.items )
      if ( !MergeWithExistingItem( visitor, itemOrdering, item.visitNow ) )
          newItems.push_back( { item.visitNow, item.hint } );

   // Choose placements for items with NEW names.

   // First sort so that like named items are together, and for the same name,
   // items with more specific ordering hints come earlier.
   std::sort( newItems.begin(), newItems.end(), Comp );

   // Outer loop over trial passes.
   int iPass = -1;
   bool force = false;
   size_t oldSize = 0;
   size_t endItemsCount = 0;
   auto prevSize = newItems.size();
   while( !newItems.empty() )
   {
      // If several items have the same hint, we try to preserve the sort by
      // name (an internal identifier, not necessarily user visible), just to
      // have some determinacy.  That requires passing one or the other way
      // over newItems.
      bool descending =
         ( iPass == OrderingHint::After || iPass == OrderingHint::Begin );

      if ( descending )
         MergeItemsDescendingNamesPass(
            visitor, itemOrdering, newItems, iPass, endItemsCount, force );
      else
         MergeItemsAscendingNamesPass(
            visitor, itemOrdering, newItems, iPass, endItemsCount, force );

      auto newSize = newItems.size();
      ++iPass;

      if ( iPass == 0 )
         // Just tried insertion by preferences.  Don't try it again.
         oldSize = newSize;
      else if ( iPass == OrderingHint::Unspecified ) {
         if ( !force ) {
            iPass = 0, oldSize = newSize;
            // Are we really ready for the final pass?
            bool progress = ( oldSize > newSize );
            if ( progress )
               // No.  While some progress is made, don't force final placements.
               // Retry Before and After hints.
               ;
            else
               force = true;
         }
      }
      else if ( iPass == OrderingHint::End && endItemsCount == 0 )
         // Remember the size before we put the ending items in place
         endItemsCount = newSize - prevSize;

      prevSize = newSize;
   }
}

// forward declaration for mutually recursive functions
void VisitItem(
   Registry::Visitor &visitor, CollectedItems &collection,
   Path &path, BaseItem *pItem,
   const GroupItem *pToMerge, const OrderingHint &hint,
   bool &doFlush );
void VisitItems(
   Registry::Visitor &visitor, CollectedItems &collection,
   Path &path, GroupItem *pGroup,
   const GroupItem *pToMerge, const OrderingHint &hint,
   bool &doFlush )
{
   // Make a NEW collection for this subtree, sharing the memo cache
   CollectedItems newCollection{ {}, collection.computedItems };

   // Gather items at this level
   // (The ordering hint is irrelevant when not merging items in)
   CollectItems( visitor, newCollection, pGroup->items, {} );

   path.push_back( pGroup->name.GET() );

   // Merge with the registry
   if ( pToMerge )
   {
      ItemOrdering itemOrdering{ path };
      newCollection.MergeItems( visitor, itemOrdering, pToMerge->items, hint );

      // Remember the NEW ordering, if there was any need to use the old.
      // This makes a side effect in preferences.
      if ( itemOrdering.gotOrdering ) {
         wxString newValue;
         for ( const auto &item : newCollection.items ) {
            const auto &name = item.visitNow->name;
            if ( !name.empty() )
               newValue += newValue.empty()
                  ? name.GET()
                  : ',' + name.GET();
         }
         if (newValue != itemOrdering.strValue) {
            gPrefs->Write( itemOrdering.key, newValue );
            doFlush = true;
         }
      }
   }

   // Now visit them
   for ( const auto &item : newCollection.items )
      VisitItem( visitor, collection, path,
         item.visitNow, item.mergeLater, item.hint,
         doFlush );

   path.pop_back();
}
void VisitItem(
   Registry::Visitor &visitor, CollectedItems &collection,
   Path &path, BaseItem *pItem,
   const GroupItem *pToMerge, const OrderingHint &hint,
   bool &doFlush )
{
   if (!pItem)
      return;

   if (const auto pSingle =
       dynamic_cast<SingleItem*>( pItem )) {
      wxASSERT( !pToMerge );
      visitor.Visit( *pSingle, path );
   }
   else
   if (const auto pGroup =
       dynamic_cast<GroupItem*>( pItem )) {
      visitor.BeginGroup( *pGroup, path );
      // recursion
      VisitItems(
         visitor, collection, path, pGroup, pToMerge, hint, doFlush );
      visitor.EndGroup( *pGroup, path );
   }
   else
      wxASSERT( false );
}

}

namespace Registry {

BaseItem::~BaseItem() {}

SharedItem::~SharedItem() {}

ComputedItem::~ComputedItem() {}

SingleItem::~SingleItem() {}

GroupItem::~GroupItem() {}

Visitor::~Visitor(){}
void Visitor::BeginGroup(GroupItem &, const Path &) {}
void Visitor::EndGroup(GroupItem &, const Path &) {}
void Visitor::Visit(SingleItem &, const Path &) {}

void Visit( Visitor &visitor, BaseItem *pTopItem, const GroupItem *pRegistry )
{
   std::vector< BaseItemSharedPtr > computedItems;
   bool doFlush = false;
   CollectedItems collection{ {}, computedItems };
   Path emptyPath;
   VisitItem(
      visitor, collection, emptyPath, pTopItem,
      pRegistry, pRegistry->orderingHint, doFlush );
   // Flush any writes done by MergeItems()
   if (doFlush)
      gPrefs->Flush();
}

OrderingPreferenceInitializer::OrderingPreferenceInitializer(
   Literal root, Pairs pairs )
   : mPairs{ std::move( pairs ) }
   , mRoot{ root }
{
   (*this)();
}

void OrderingPreferenceInitializer::operator () ()
{
   bool doFlush = false;
   for (const auto &pair : mPairs) {
      const auto key = wxString{'/'} + mRoot + pair.first;
      if ( gPrefs->Read(key).empty() ) {
         gPrefs->Write( key, pair.second );
         doFlush = true;
      }
   }
   
   if (doFlush)
      gPrefs->Flush();
}

void RegisterItem( GroupItem &registry, const Placement &placement,
   BaseItemPtr pItem )
{
   // Since registration determines only an unordered tree of menu items,
   // we can sort children of each node lexicographically for our convenience.
   BaseItemPtrs *pItems;
   struct Comparator {
      bool operator()
         ( const Identifier &component, const BaseItemPtr& pItem ) const {
            return component < pItem->name; }
      bool operator()
         ( const BaseItemPtr& pItem, const Identifier &component ) const {
            return pItem->name < component; }
   };
   auto find = [&pItems]( const Identifier &component ){ return std::equal_range(
      pItems->begin(), pItems->end(), component, Comparator() ); };

   auto pNode = &registry;
   pItems = &pNode->items;

   const auto pathComponents = ::wxSplit( placement.path, '/' );
   auto pComponent = pathComponents.begin(), end = pathComponents.end();

   // Descend the registry hierarchy, while groups matching the path components
   // can be found
   auto debugPath = wxString{'/'} + registry.name.GET();
   while ( pComponent != end ) {
      const auto &pathComponent = *pComponent;

      // Try to find an item already present that is a group item with the
      // same name; we don't care which if there is more than one.
      const auto range = find( pathComponent );
      const auto iter2 = std::find_if( range.first, range.second,
         [](const BaseItemPtr &pItem){
            return dynamic_cast< GroupItem* >( pItem.get() ); } );

      if ( iter2 != range.second ) {
         // A matching group in the registry, so descend
         pNode = static_cast< GroupItem* >( iter2->get() );
         pItems = &pNode->items;
         debugPath += '/' + pathComponent;
         ++pComponent;
      }
      else
         // Insert at this level;
         // If there are no more path components, and a name collision of
         // the added item with something already in the registry, don't resolve
         // it yet in this function, but see MergeItems().
         break;
   }

   // Create path group items for remaining components
   while ( pComponent != end ) {
      auto newNode = std::make_unique<TransparentGroupItem<>>( *pComponent );
      pNode = newNode.get();
      pItems->insert( find( pNode->name ).second, std::move( newNode ) );
      pItems = &pNode->items;
      ++pComponent;
   }

   // Remember the hint, to be used later in merging.
   pItem->orderingHint = placement.hint;

   // Now insert the item.
   pItems->insert( find( pItem->name ).second, std::move( pItem ) );
}

}
