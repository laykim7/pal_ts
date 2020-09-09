importPackage(Packages.org.csstudio.opibuilder.scriptUtil);
importPackage(Packages.org.csstudio.platform.data);

var CBox_27 = display.getWidget("Check Box_27")
var CBox_28 = display.getWidget("Check Box_28")
var TInput_19 = display.getWidget("Text Update_22")

var value27 = CBox_27.getValue()
var value28 = CBox_28.getValue()

TInput_19.setPropertyValue("text",value27);












var combo = display.getWidget("Combo_7")
                                                     
var str = combo.getValue()                           
                                                     
if(str == "User Trigger")       pvs[1].setValue(0 ); 
else if (str == "MXC_00")       pvs[1].setValue(1 ); 
else if (str == "MXC_01")       pvs[1].setValue(2 ); 
else if (str == "MXC_02")       pvs[1].setValue(3 ); 
else if (str == "MXC_03")       pvs[1].setValue(4 ); 
else if (str == "MXC_04")       pvs[1].setValue(5 ); 
else if (str == "MXC_05")       pvs[1].setValue(6 ); 
else if (str == "MXC_06")       pvs[1].setValue(7 ); 
else if (str == "MXC_07")       pvs[1].setValue(8 ); 
else if (str == "MXC_08")       pvs[1].setValue(9 ); 
else if (str == "MXC_09")       pvs[1].setValue(10); 
else if (str == "MXC_10")       pvs[1].setValue(11); 
else if (str == "MXC_11")       pvs[1].setValue(12); 
else if (str == "MXC_12")       pvs[1].setValue(13); 
else if (str == "MXC_13")       pvs[1].setValue(14); 
else if (str == "EXT_IN_SL_00") pvs[1].setValue(15); 
else if (str == "EXT_IN_SL_01") pvs[1].setValue(16); 
else if (str == "EXT_IN_SL_02") pvs[1].setValue(17); 
else if (str == "EXT_IN_SL_03") pvs[1].setValue(18); 
else if (str == "EXT_IN_SL_04") pvs[1].setValue(19); 
else if (str == "EXT_IN_SL_05") pvs[1].setValue(20); 
else if (str == "EXT_IN_SL_06") pvs[1].setValue(21); 
else if (str == "EXT_IN_SL_07") pvs[1].setValue(22); 
else if (str == "EXT_IN_SL_08") pvs[1].setValue(23); 
else if (str == "EXT_IN_SL_09") pvs[1].setValue(24); 
else if (str == "EXT_IN_SL_10") pvs[1].setValue(25); 
else if (str == "EXT_IN_SL_11") pvs[1].setValue(26); 
else if (str == "EXT_IN_SL_12") pvs[1].setValue(27); 
else if (str == "EXT_IN_SL_13") pvs[1].setValue(28); 
else if (str == "EXT_IN_SL_14") pvs[1].setValue(29); 
else if (str == "EXT_IN_SL_15") pvs[1].setValue(30); 
else if (str == "EXT_IN_SH_00") pvs[1].setValue(31); 
else if (str == "EXT_IN_SH_01") pvs[1].setValue(32); 
else if (str == "EXT_IN_SH_02") pvs[1].setValue(33); 
else if (str == "EXT_IN_SH_03") pvs[1].setValue(34); 
else if (str == "EXT_IN_SH_04") pvs[1].setValue(35); 
else if (str == "EXT_IN_SH_05") pvs[1].setValue(36); 
else if (str == "EXT_IN_SH_06") pvs[1].setValue(37); 
else if (str == "EXT_IN_SH_07") pvs[1].setValue(38); 
else if (str == "EXT_IN_SH_08") pvs[1].setValue(39); 
else if (str == "EXT_IN_SH_09") pvs[1].setValue(40); 
else if (str == "EXT_IN_SH_10") pvs[1].setValue(41); 
else if (str == "EXT_IN_SH_11") pvs[1].setValue(42); 
else if (str == "EXT_IN_SH_12") pvs[1].setValue(43); 
else if (str == "EXT_IN_SH_13") pvs[1].setValue(44); 
else if (str == "EXT_IN_SH_14") pvs[1].setValue(45); 
else if (str == "EXT_IN_SH_15") pvs[1].setValue(46); 
