importPackage(Packages.org.csstudio.opibuilder.scriptUtil);

cb1Val = display.getWidget("comboBox_1").getValue();
cb2Val = display.getWidget("comboBox_2").getValue();
cb3Val = display.getWidget("comboBox_5").getValue();
cb4Val = display.getWidget("comboBox_4").getValue();

var macroInput = DataUtil.createMacrosInput(true);
macroInput.put("SYS", cb1Val);
macroInput.put("SUBSYS", cb2Val);
macroInput.put("DEV", cb3Val + cb4Val);
	
ScriptUtil.openOPI(widget, "zqsystem.opi", 0, macroInput);






