/**********************************************************************

Audacium: A Digital Audio Editor

UIHandle.cpp

Paul Licameli

**********************************************************************/


#include "UIHandle.h"

#include "RefreshCode.h"

UIHandle::~UIHandle()
{
}

void UIHandle::Enter(bool, AudaciumProject *)
{
}

bool UIHandle::HasRotation() const
{
   return false;
}

bool UIHandle::Rotate(bool)
{
   return false;
}

bool UIHandle::HasEscape() const
{
   return false;
}

bool UIHandle::Escape(AudaciumProject *)
{
   return false;
}

bool UIHandle::StopsOnKeystroke()
{
   return false;
}

void UIHandle::OnProjectChange(AudaciumProject *)
{
}
