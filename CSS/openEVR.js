importPackage(Packages.org.csstudio.opibuilder.scriptUtil);

comboValue = display.getWidget("comboBox").getValue();

var macroInput = DataUtil.createMacrosInput(true);
macroInput.put("SYS", comboValue);
macroInput.put("SUBSYS", "TS");
	
ScriptUtil.openOPI(widget, "EVR.opi", 0, macroInput);






