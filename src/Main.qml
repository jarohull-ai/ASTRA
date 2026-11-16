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
    ToolButton {text:backend.paused?qsTr("Wznów"):qsTr("Pauza");onClicked:backend.paused=!backend.paused}
    Button {text:win.editing?qsTr("Gotowe"):qsTr("Edytuj układ");highlighted:win.editing;onClicked:win.editing=!win.editing}
    ToolButton {text:"⚙";font.pixelSize:22;onClicked:settingsDrawer.open();ToolTip.text:qsTr("Wygląd i kafelki");ToolTip.visible:hovered}
   }
   Rectangle {Layout.fillWidth:true;height:1;color:"#25414a"}
   RowLayout {Layout.fillWidth:true
    ColumnLayout {spacing:5
     Text {text:qsTr("Centrum dowodzenia");color:"#e9f4f8";font.pixelSize:29;font.weight:Font.Light}
     Text {text:win.editing?qsTr("Strzałki zmieniają kolejność · × usuwa kafelek · ustawienia dodają kafelki"):qsTr("Twój system. Jeden widok. Pełna kontrola.");color:"#7999a5";font.pixelSize:12;wrapMode:Text.WordWrap;Layout.maximumWidth:win.width-150}
    }
    Item {Layout.fillWidth:true}
    Rectangle {implicitWidth:102;implicitHeight:28;radius:14;color:"#122d2b";border.color:"#285a50";visible:win.width>950
     Text {anchors.centerIn:parent;text:backend.paused?qsTr("●  PAUZA"):qsTr("●  ONLINE");color:win.accent;font.pixelSize:10;font.letterSpacing:1}
    }
   }
   ScrollView {id:scroll;Layout.fillWidth:true;Layout.fillHeight:true;clip:true;contentWidth:availableWidth
    GridLayout {width:scroll.availableWidth;columns:win.width>=1040?3:2;columnSpacing:16;rowSpacing:16
     Repeater {model:backend.order
      TelemetryTile {
       required property string modelData
       required property int index
       Layout.fillWidth:true;Layout.preferredWidth:300;Layout.preferredHeight:Math.max(220,(scroll.availableHeight-16)/2)
       metric:{win.revision;return backend.card(modelData)}
       accent:win.accent;glass:backend.glass;editing:win.editing;serial:index
       onRemoveRequested:backend.toggle(modelData)
       onMoveRequested:function(direction){backend.move(modelData,direction)}
      }
     }
     Text {visible:backend.order.length===0;text:qsTr("Dodaj kafelki w ustawieniach ⚙");color:"#aec7d1";font.pixelSize:20;Layout.topMargin:40}
    }
   }
   RowLayout {Layout.fillWidth:true
    Text {text:qsTr("LOCAL SENSORS  /  1 Hz");color:"#678893";font.family:"monospace";font.pixelSize:10}
    Item {Layout.fillWidth:true}
    Text {text:qsTr("E  układ     SPACE  pauza     ?  pomoc");color:"#678893";font.pixelSize:10}
   }
  }
 }
 Drawer {
  id:settingsDrawer;edge:Qt.RightEdge;width:330;height:win.height
  background:Rectangle {color:"#101c24";border.color:"#36515c"}
  ColumnLayout {anchors.fill:parent;anchors.margins:24;spacing:14
   Text {text:qsTr("Twoja konsola");color:"#e6f7ff";font.pixelSize:24}
   Text {text:qsTr("Widoczne kafelki");color:win.accent;font.pixelSize:13}
   Repeater {model:[{key:"cpu",name:qsTr("Procesor")},{key:"memory",name:qsTr("Pamięć RAM")},{key:"gpu",name:qsTr("Grafika")},{key:"thermal",name:qsTr("Temperatura CPU")},{key:"disk",name:qsTr("Dysk systemowy")},{key:"network",name:qsTr("Sieć")}]
    CheckBox {required property var modelData;text:modelData.name;checked:backend.order.indexOf(modelData.key)>=0;onClicked:backend.toggle(modelData.key);Layout.preferredHeight:32}
   }
   Text {text:qsTr("Kolor światła");color:win.accent;font.pixelSize:13}
   ComboBox {Layout.fillWidth:true;model:["Omarchy","Ice blue","Amber"];currentIndex:["omarchy","ice","amber"].indexOf(backend.palette);onActivated:backend.palette=["omarchy","ice","amber"][currentIndex]}
   Text {text:qsTr("Gęstość szkła");color:win.accent;font.pixelSize:13}
   Slider {Layout.fillWidth:true;from:.45;to:.98;value:backend.glass;onMoved:backend.glass=value}
   Text {text:qsTr("Przezroczystość jest rzeczywista. Rozmycie tła zależy od ustawień Hyprlanda.");color:"#8eaab5";font.pixelSize:12;wrapMode:Text.WordWrap;Layout.fillWidth:true}
   Button {text:qsTr("Przywróć domyślny układ");onClicked:backend.reset()}
   Item {Layout.fillHeight:true}
   Button {text:qsTr("Zamknij ustawienia");onClicked:settingsDrawer.close()}
  }
 }
 Popup {id:help;anchors.centerIn:parent;width:380;modal:true;padding:24
  ColumnLayout {spacing:12;Text {text:qsTr("Sterowanie ASTRA");color:"white";font.pixelSize:22} Text {text:qsTr("E — edytuj kolejność i usuń kafelki\nSpace — wstrzymaj odczyty\nQ — zamknij\n⚙ — dodaj kafelki, zmień kolor i szkło\n\nUkład i wygląd zapisują się automatycznie.");color:"#b9d0d9";font.pixelSize:13} Button {text:qsTr("Rozumiem");onClicked:help.close()}}
 }
}
