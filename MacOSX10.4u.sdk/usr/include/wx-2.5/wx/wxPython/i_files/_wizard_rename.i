// A bunch of %rename directives generated by BuildRenamers in config.py
// in order to remove the wx prefix from all global scope names.

#ifndef BUILDING_RENAMERS

%rename(WIZARD_EX_HELPBUTTON)               wxWIZARD_EX_HELPBUTTON;
%rename(WizardEvent)                        wxWizardEvent;
%rename(WizardPage)                         wxWizardPage;
%rename(PyWizardPage)                       wxPyWizardPage;
%rename(WizardPageSimple)                   wxWizardPageSimple;
%rename(Wizard)                             wxWizard;

#endif