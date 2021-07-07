/**********************************************************************

   Audacium: A Digital Audio Editor
   Audacium(R) is copyright (c) 1999-2018 Audacium Team.
   File License: wxwidgets

   OpenSaveCommands.h
   Stephen Parry
   James Crook

******************************************************************//**

\class OpenProjectCommand
\brief Command for opening an Audacium project

\class SaveProjectCommand
\brief Command for saving an Audacium project

\class SaveCopyCommand
\brief Command for saving a copy of currently project

\class SaveLogCommand
\brief Command for saving the log contents

*//*******************************************************************/

#include "Command.h"
#include "CommandType.h"

class OpenProjectCommand : public AudaciumCommand
{
public:
   static const ComponentInterfaceSymbol Symbol;

   // ComponentInterface overrides
   ComponentInterfaceSymbol GetSymbol() override {return Symbol;};
   TranslatableString GetDescription() override {return XO("Opens a project.");};
   bool DefineParams( ShuttleParams & S ) override;
   void PopulateOrExchange(ShuttleGui & S) override;
   bool Apply(const CommandContext & context) override;

   // AudaciumCommand overrides
   ManualPageID ManualPage() override {return L"Extra_Menu:_Scriptables_II#open_project";}
public:
   wxString mFileName;
   bool mbAddToHistory;
   bool bHasAddToHistory;
};

class SaveProjectCommand : public AudaciumCommand
{
public:
   static const ComponentInterfaceSymbol Symbol;

   // ComponentInterface overrides
   ComponentInterfaceSymbol GetSymbol() override {return Symbol;};
   TranslatableString GetDescription() override {return XO("Saves a project.");};
   bool DefineParams( ShuttleParams & S ) override;
   void PopulateOrExchange(ShuttleGui & S) override;
   bool Apply(const CommandContext & context) override;

   // AudaciumCommand overrides
   ManualPageID ManualPage() override {return L"Extra_Menu:_Scriptables_II#save_project";}
public:
   wxString mFileName;
   bool mbAddToHistory;
   bool bHasAddToHistory;
};

class SaveCopyCommand : public AudaciumCommand
{
public:
   static const ComponentInterfaceSymbol Symbol;

   // ComponentInterface overrides
   ComponentInterfaceSymbol GetSymbol() override {return Symbol;};
   TranslatableString GetDescription() override {return XO("Saves a copy of current project.");};
   bool DefineParams( ShuttleParams & S ) override;
   void PopulateOrExchange(ShuttleGui & S) override;
   bool Apply(const CommandContext & context) override;

   // AudaciumCommand overrides
   ManualPageID ManualPage() override {return L"Extra_Menu:_Scriptables_II#save_copy";}
public:
   wxString mFileName;
};

class SaveLogCommand : public AudaciumCommand
{
public:
   static const ComponentInterfaceSymbol Symbol;

   // ComponentInterface overrides
   ComponentInterfaceSymbol GetSymbol() override {return Symbol;};
   TranslatableString GetDescription() override {return XO("Saves the log contents.");};
   bool DefineParams( ShuttleParams & S ) override;
   void PopulateOrExchange(ShuttleGui & S) override;
   bool Apply(const CommandContext & context) override;

   // AudaciumCommand overrides
   ManualPageID ManualPage() override {return L"Extra_Menu:_Scriptables_II#save_log";}
public:
   wxString mFileName;
};

class ClearLogCommand : public AudaciumCommand
{
public:
   static const ComponentInterfaceSymbol Symbol;

   // ComponentInterface overrides
   ComponentInterfaceSymbol GetSymbol() override {return Symbol;};
   TranslatableString GetDescription() override {return XO("Clears the log contents.");};
   bool DefineParams( ShuttleParams & S ) override;
   bool PromptUser(wxWindow *parent) override;
   bool Apply(const CommandContext & context) override;

   // AudaciumCommand overrides
   ManualPageID ManualPage() override {return L"Extra_Menu:_Scriptables_II#Clear_log";}
public:
   wxString mFileName;
};
