/**********************************************************************

Audacium: A Digital Audio Editor

ProjectWindowBase.h

Paul Licameli split from ProjectWindow.h

**********************************************************************/

#ifndef __AUDACITY_PROJECT_WINDOW_BASE__
#define __AUDACITY_PROJECT_WINDOW_BASE__

#include <wx/frame.h> // to inherit

class AudaciumProject;

///\brief A top-level window associated with a project
class ProjectWindowBase /* not final */ : public wxFrame
{
public:
   explicit ProjectWindowBase(
      wxWindow * parent, wxWindowID id,
      const wxPoint & pos, const wxSize &size,
      AudaciumProject &project );

   ~ProjectWindowBase() override;

   AudaciumProject &GetProject() { return mProject; }
   const AudaciumProject &GetProject() const { return mProject; }

protected:
   AudaciumProject &mProject;
};

AUDACITY_DLL_API AudaciumProject *FindProjectFromWindow( wxWindow *pWindow );
const AudaciumProject *FindProjectFromWindow( const wxWindow *pWindow );

#endif

