/**********************************************************************

Audacium: A Digital Audio Editor

@file AudaciumFileConfig.h
@brief Extend FileConfig with application-specific behavior

Paul Licameli split from Prefs.h

**********************************************************************/

#ifndef __AUDACITY_FILE_CONFIG__
#define __AUDACITY_FILE_CONFIG__

#include <memory>
#include "widgets/FileConfig.h" // to inherit

/// \brief Our own specialisation of FileConfig.
class AUDACITY_DLL_API AudaciumFileConfig final : public FileConfig
{
public:
   //! Require a call to this factory, to guarantee proper two-phase initialization
   static std::unique_ptr<AudaciumFileConfig> Create(
      const wxString& appName = {},
      const wxString& vendorName = {},
      const wxString& localFilename = {},
      const wxString& globalFilename = {},
      long style = wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_GLOBAL_FILE,
      const wxMBConv& conv = wxConvAuto()
   );

   ~AudaciumFileConfig() override;

protected:
   void Warn() override;

private:
   //! Disallow direct constructor call, because a two-phase initialization is required
   AudaciumFileConfig(
      const wxString& appName,
      const wxString& vendorName,
      const wxString& localFilename,
      const wxString& globalFilename,
      long style,
      const wxMBConv& conv
   );
};
#endif
