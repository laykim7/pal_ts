importPackage(Packages.org.csstudio.opibuilder.scriptUtil);
importPackage(Packages.org.eclipse.jface.dialogs);
importPackage(Packages.java.lang);


var value = PVUtil.getLong(pvs[0]);
var value1 = (value>>>16) & 65535;
var value2 = (value>>>0) & 65535;

// ConsoleUtil.writeInfo(value);
// ConsoleUtil.writeInfo(value1);
// ConsoleUtil.writeInfo(value2);

ConsoleUtil.writeInfo( (pvs[1]) );

var graph1 = display.getWidget("CNTR_A")
var graph2 = display.getWidget("CNTR_B")

graph1.setPropertyValue("text", value1.toString(10));
graph2.setPropertyValue("text", value2.toString(10));

// widget.setPropertyValue("text", "0x"+ value1.toString(16) );
