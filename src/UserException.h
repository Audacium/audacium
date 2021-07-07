/*!
 @file UserException.h
 @brief An AudaciumException with no visible message

 Created by Paul Licameli on 11/27/16.

*/

#ifndef __AUDACITY_USER_EXCEPTION__
#define __AUDACITY_USER_EXCEPTION__

#include "AudaciumException.h"

 //! Can be thrown when user cancels operations, as with a progress dialog.  Delayed handler does nothing
/*! This class does not inherit from MessageBoxException. */
class AUDACITY_DLL_API UserException final : public AudaciumException
{
public:
   UserException() {}

   ~UserException() override;

   void DelayedHandlerAction() override;
};

#endif
