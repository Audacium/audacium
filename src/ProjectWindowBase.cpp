/**********************************************************************

Audacium: A Digital Audio Editor

ProjectWindowBase.cpp

Paul Licameli split from ProjectWindow.cpp

**********************************************************************/

#include "ProjectWindowBase.h"

#include "Project.h"

ProjectWindowBase::ProjectWindowBase(wxWindow * parent, wxWindowID id,
                                 const wxPoint & pos,
                                 const wxSize & size, AudaciumProject &project)
   : wxFrame(parent, id, _TS("Audacium"), pos, size)
   , mProject{ project }
{
   project.SetFrame( this );
};

ProjectWindowBase::~ProjectWindowBase()
{
}

namespace {

ProjectWindowBase *FindProjectWindow( wxWindow *pWindow )
{
   while ( pWindow && pWindow->GetParent() )
      pWindow = pWindow->GetParent();
   return dynamic_cast< ProjectWindowBase* >( pWindow );
}

}

AudaciumProject *FindProjectFromWindow( wxWindow *pWindow )
{
   auto pProjectWindow = FindProjectWindow( pWindow );
   return pProjectWindow ? &pProjectWindow->GetProject() : nullptr;
}

const AudaciumProject *FindProjectFromWindow( const wxWindow *pWindow )
{
   return FindProjectFromWindow( const_cast< wxWindow* >( pWindow ) );
}

