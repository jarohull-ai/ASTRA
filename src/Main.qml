import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
ApplicationWindow {
 id: win
 width:1180;height:850;minimumWidth:700;minimumHeight:540
 visible:true;title:"ASTRA · System telemetry";color:"transparent"
 Material.theme:Material.Dark;Material.accent:backend.accent
 property bool editing:false
 property int revision:0
 property string clockText:""
 property color accent:backend.accent
 Connections {target:backend;function onChanged(){win.revision++}}
 Timer {interval:1000;running:true;repeat:true;triggeredOnStart:true;onTriggered:win.clockText=new Date().toLocaleTimeString(Qt.locale(),"HH:mm:ss")}
 Shortcut {sequence:"Q";onActivated:Qt.quit()}
 Shortcut {sequence:"Space";onActivated:backend.paused=!backend.paused}
 Shortcut {sequence:"E";onActivated:win.editing=!win.editing}
 Shortcut {sequence:"?";onActivated:help.open()}
 Rectangle {
  anchors.fill:parent;radius:22;color:Qt.rgba(.018,.035,.055,backend.glass);border.color:"#31444d"
  clip:true
  Rectangle {x:parent.width*.45;y:-400;width:850;height:850;radius:425;opacity:.10;gradient:Gradient{GradientStop{position:0;color:win.accent}GradientStop{position:1;color:"transparent"}}}
  Rectangle {x:-300;y:parent.height-300;width:750;height:750;radius:375;opacity:.08;gradient:Gradient{GradientStop{position:0;color:"#588bff"}GradientStop{position:1;color:"transparent"}}}
  Canvas {anchors.fill:parent;opacity:.17;onWidthChanged:requestPaint();onHeightChanged:requestPaint();onPaint:{var c=getContext("2d");c.reset();c.fillStyle="#5f8797";for(var x=18;x<width;x+=28)for(var y=18;y<height;y+=28)c.fillRect(x,y,1,1);}}
  ColumnLayout {
   anchors.fill:parent;anchors.margins:32;spacing:18
   RowLayout {Layout.fillWidth:true;spacing:16
    Rectangle {width:42;height:42;radius:12;color:"#163136";border.color:win.accent
     Text {anchors.centerIn:parent;text:"Λ";font.pixelSize:30;font.weight:Font.Light;color:win.accent}
    }
    ColumnLayout {spacing:2
     Text {text:"A S T R A";font.pixelSize:23;font.weight:Font.DemiBold;color:"#eff9ff"}
     Text {text:qsTr("SYSTEM TELEMETRY / PERSONAL CONSOLE");font.pixelSize:9;font.letterSpacing:1.6;color:"#7898a5"}
    }
    Item {Layout.fillWidth:true}
    Text {text:win.clockText;color:"#9bb8c2";font.family:"monospace";font.pixelSize:15;visible:win.width>850}
    ToolButton {text:backend.paused?qsTr("Resume"):qsTr("Pause");onClicked:backend.paused=!backend.paused}
    Button {text:win.editing?qsTr("Done"):qsTr("Edit layout");highlighted:win.editing;onClicked:win.editing=!win.editing}
    ToolButton {text:"⚙";font.pixelSize:22;onClicked:settingsDrawer.open();ToolTip.text:qsTr("Appearance and tiles");ToolTip.visible:hovered}
   }
   Rectangle {Layout.fillWidth:true;height:1;color:"#25414a"}
   RowLayout {Layout.fillWidth:true
    ColumnLayout {spacing:5
     Text {text:qsTr("Command center");color:"#e9f4f8";font.pixelSize:29;font.weight:Font.Light}
     Text {text:win.editing?qsTr("Arrows reorder · × removes a tile · settings add tiles"):qsTr("Your system. One view. Full control.");color:"#7999a5";font.pixelSize:12;wrapMode:Text.WordWrap;Layout.maximumWidth:win.width-150}
    }
    Item {Layout.fillWidth:true}
    Rectangle {implicitWidth:102;implicitHeight:28;radius:14;color:"#122d2b";border.color:"#285a50";visible:win.width>950
     Text {anchors.centerIn:parent;text:backend.paused?qsTr("●  PAUSED"):qsTr("●  ONLINE");color:win.accent;font.pixelSize:10;font.letterSpacing:1}
    }
   }
   ScrollView {id:scroll;Layout.fillWidth:true;Layout.fillHeight:true;clip:true;contentWidth:availableWidth
    GridLayout {width:scroll.availableWidth;columns:win.width>=1040?3:2;columnSpacing:16;rowSpacing:16
     Repeater {model:backend.order
      TelemetryTile {
       required property string modelData
       required property int index
       Layout.fillWidth:true;Layout.preferredWidth:300;Layout.preferredHeight:Math.max(184,(scroll.availableHeight-16)/2)
       metric:{win.revision;return backend.card(modelData)}
       accent:win.accent;glass:backend.glass;editing:win.editing;serial:index
       onRemoveRequested:backend.toggle(modelData)
       onMoveRequested:function(direction){backend.move(modelData,direction)}
      }
     }
     Text {visible:backend.order.length===0;text:qsTr("Add tiles in settings ⚙");color:"#aec7d1";font.pixelSize:20;Layout.topMargin:40}
    }
   }
   ParadoxPanel {Layout.fillWidth:true;Layout.preferredHeight:136;s:paradox.state;accent:win.accent;glass:backend.glass}
   RowLayout {Layout.fillWidth:true
    Text {text:qsTr("LOCAL SENSORS  /  1 Hz");color:"#678893";font.family:"monospace";font.pixelSize:10}
    Item {Layout.fillWidth:true}
    Text {text:qsTr("E  layout     SPACE  pause     ?  help");color:"#678893";font.pixelSize:10}
   }
  }
 }
 Drawer {
  id:settingsDrawer;edge:Qt.RightEdge;width:330;height:win.height
  background:Rectangle {color:"#101c24";border.color:"#36515c"}
  ColumnLayout {anchors.fill:parent;anchors.margins:24;spacing:14
   Text {text:qsTr("Your console");color:"#e6f7ff";font.pixelSize:24}
   Text {text:qsTr("Visible tiles");color:win.accent;font.pixelSize:13}
   Repeater {model:[{key:"cpu",name:qsTr("Processor")},{key:"memory",name:qsTr("Memory")},{key:"gpu",name:qsTr("Graphics")},{key:"thermal",name:qsTr("CPU temperature")},{key:"disk",name:qsTr("System disk")},{key:"network",name:qsTr("Network")}]
    CheckBox {required property var modelData;text:modelData.name;checked:backend.order.indexOf(modelData.key)>=0;onClicked:backend.toggle(modelData.key);Layout.preferredHeight:32}
   }
   Text {text:qsTr("Language");color:win.accent;font.pixelSize:13}
   ComboBox {id:langBox;Layout.fillWidth:true;readonly property var codes:["auto","en","pl"]
    model:[qsTr("Automatic (Omarchy)"),"English","Polski"]
    currentIndex:Math.max(0,codes.indexOf(i18n.choice))
    onActivated:i18n.choice=codes[currentIndex]}
   Text {text:qsTr("Light colour");color:win.accent;font.pixelSize:13}
   ComboBox {Layout.fillWidth:true;model:["Omarchy",qsTr("Ice blue"),qsTr("Amber")];currentIndex:["omarchy","ice","amber"].indexOf(backend.palette);onActivated:backend.palette=["omarchy","ice","amber"][currentIndex]}
   Text {text:qsTr("Glass density");color:win.accent;font.pixelSize:13}
   Slider {Layout.fillWidth:true;from:.45;to:.98;value:backend.glass;onMoved:backend.glass=value}
   Text {text:qsTr("Transparency is real. Background blur depends on your Hyprland settings.");color:"#8eaab5";font.pixelSize:12;wrapMode:Text.WordWrap;Layout.fillWidth:true}
   Button {text:qsTr("Restore default layout");onClicked:backend.reset()}
   Item {Layout.fillHeight:true}
   Button {text:qsTr("Close settings");onClicked:settingsDrawer.close()}
  }
 }
 Popup {id:help;anchors.centerIn:parent;width:380;modal:true;padding:24
  ColumnLayout {spacing:12;Text {text:qsTr("ASTRA controls");color:"white";font.pixelSize:22} Text {text:qsTr("E — reorder and remove tiles\nSpace — pause readings\nQ — quit\n⚙ — add tiles, change colour, glass and language\n\nLayout and appearance are saved automatically.");color:"#b9d0d9";font.pixelSize:13} Button {text:qsTr("Got it");onClicked:help.close()}}
 }
}
