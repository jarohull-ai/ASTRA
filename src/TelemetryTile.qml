import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
 id: tile
 property var metric: ({})
 property color accent: "#31f7a5"
 property bool editing: false
 property real glass: .83
 property int serial: 0
 signal removeRequested()
 signal moveRequested(int direction)
 radius: 18
 color: Qt.rgba(.035,.065,.085,glass)
 border.color: Qt.rgba(accent.r,accent.g,accent.b,.20)
 border.width: 1
 clip: true
 Rectangle {anchors.fill: parent;radius: parent.radius;gradient: Gradient {GradientStop {position:0;color:"#15bce4ed"} GradientStop {position:.48;color:"#027cabb5"} GradientStop {position:1;color:"#0a000000"}}}
 Rectangle {x:22;y:0;width:parent.width-44;height:1;color:Qt.rgba(.8,1,1,.25)}
 Rectangle {x:22;y:0;width:42;height:2;color:tile.accent}
 ColumnLayout {
  anchors.fill: parent;anchors.margins:tile.height<210?16:22;spacing:tile.height<210?3:5
  RowLayout {
   Layout.fillWidth:true
   Text {text: ("0"+(tile.serial+1)).slice(-2);color:tile.accent;font.family:"monospace";font.pixelSize:11}
   Text {text:tile.metric.label||"";color:"#b4ced5";font.pixelSize:13;font.letterSpacing:1;Layout.fillWidth:true;textFormat:Text.PlainText}
   Rectangle {width:5;height:5;radius:3;color:tile.metric.available?tile.accent:"#6b7683";visible:!tile.editing}
   ToolButton {text:"‹";visible:tile.editing;implicitWidth:28;implicitHeight:28;onClicked:tile.moveRequested(-1);ToolTip.text:qsTr("Przesuń wcześniej");ToolTip.visible:hovered}
   ToolButton {text:"›";visible:tile.editing;implicitWidth:28;implicitHeight:28;onClicked:tile.moveRequested(1);ToolTip.text:qsTr("Przesuń dalej");ToolTip.visible:hovered}
   ToolButton {text:"×";visible:tile.editing;implicitWidth:28;implicitHeight:28;onClicked:tile.removeRequested();ToolTip.text:qsTr("Usuń kafelek");ToolTip.visible:hovered}
  }
  RowLayout {spacing:8;Layout.topMargin:tile.height<210?0:8
   Text {text:tile.metric.value||"—";font.pixelSize:tile.height<210?34:tile.height<250?42:50;font.weight:Font.Light;font.letterSpacing:-2;color:"#effaff"}
   Text {text:tile.metric.unit||"";font.pixelSize:14;color:tile.accent;Layout.alignment:Qt.AlignBottom;Layout.bottomMargin:tile.height<210?8:12}
   Item {Layout.fillWidth:true}
  }
  Text {text:tile.metric.detail||"";font.pixelSize:11;color:"#8ca6b0";Layout.fillWidth:true;elide:Text.ElideRight;textFormat:Text.PlainText;ToolTip.text:text;ToolTip.visible:detailHover.hovered;HoverHandler{id:detailHover}}
  Item {Layout.fillHeight:true;Layout.minimumHeight:tile.height<210?2:8}
  Canvas {
   id: graph;Layout.fillWidth:true;Layout.preferredHeight:tile.height<210?32:tile.height<250?45:67
   property var samples:tile.metric.history||[]
   onSamplesChanged:requestPaint()
   onWidthChanged:requestPaint()
   onPaint:{
    var c=getContext("2d");c.reset();c.strokeStyle="#143641";c.lineWidth=1;
    for(var j=0;j<4;j++){var y=4+j*(height-8)/3;c.beginPath();c.moveTo(0,y);c.lineTo(width,y);c.stroke();}
    for(var k=0;k<7;k++){var x=k*width/6;c.beginPath();c.moveTo(x,0);c.lineTo(x,height);c.stroke();}
    if(samples.length<2)return;
    var ceiling=tile.metric.id==="network"?Math.max(.01,Math.max.apply(null,samples)*1.2):100;
    var step=width/89,first=width-(samples.length-1)*step;
    function yy(v){return height-4-Math.min(1,Math.max(0,v/ceiling))*(height-8);}
    c.beginPath();c.moveTo(first,height);for(var i=0;i<samples.length;i++)c.lineTo(first+i*step,yy(samples[i]));c.lineTo(width,height);c.closePath();
    var g=c.createLinearGradient(0,0,0,height);g.addColorStop(0,Qt.rgba(tile.accent.r,tile.accent.g,tile.accent.b,.28));g.addColorStop(1,"#0031f7a5");c.fillStyle=g;c.fill();
    c.beginPath();for(var n=0;n<samples.length;n++){var px=first+n*step,py=yy(samples[n]);if(n===0)c.moveTo(px,py);else c.lineTo(px,py);}c.strokeStyle=tile.accent;c.lineWidth=1.8;c.stroke();
    c.beginPath();c.arc(width-2,yy(samples[samples.length-1]),3,0,Math.PI*2);c.fillStyle="#eafffa";c.fill();
   }
  }
  RowLayout {Layout.fillWidth:true
   Text {text:qsTr("HISTORIA / 90 s");font.family:"monospace";font.pixelSize:9;font.letterSpacing:1;color:"#52737e"}
   Item {Layout.fillWidth:true}
   Text {text:tile.metric.available?qsTr("LIVE"):qsTr("BRAK DANYCH");font.family:"monospace";font.pixelSize:9;color:tile.metric.available?tile.accent:"#87969b"}
  }
 }
}
