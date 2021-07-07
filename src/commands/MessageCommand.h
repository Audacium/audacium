/**********************************************************************

   Audacium - A Digital Audio Editor
   Copyright 1999-2009 Audacium Team
   File License: wxWidgets

   Dan Horgan

******************************************************************//**

\file MessageCommand.h
\brief Contains definition of MessageCommand class.

*//***************************************************************//**

\class MessageCommand
\brief Command to send a message (currently on the status channel)

*//*******************************************************************/

#ifndef __MESSAGE_COMMAND__
#define __MESSAGE_COMMAND__

#include "CommandType.h"
#include "Command.h"

class MessageCommand : public AudaciumCommand
{
public:
   static const ComponentInterfaceSymbol Symbol;

   // ComponentInterface overrides
   ComponentInterfaceSymbol GetSymbol() override {return Symbol;};
   TranslatableString GetDescription() override {return XO("Echos a message.");};
   bool DefineParams( ShuttleParams & S ) override;
   void PopulateOrExchange(ShuttleGui & S) override;
   bool Apply(const CommandContext & context) override;

   // AudaciumCommand overrides
   ManualPageID ManualPage() override {return L"Extra_Menu:_Scriptables_II#message";}
public:
   wxString mMessage;
};


#endif /* End of include guard: __MESSAGECOMMAND__ */
