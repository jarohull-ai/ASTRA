import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
// JFP PARADOX strip — same glass language as TelemetryTile.
Rectangle {
 id: panel
 property var s: ({})
 property color accent: "#31f7a5"
 property real glass: .83
 readonly property bool ready: s.ready===true
 readonly property color stateColor: !ready?"#6b7683":s.state==="critical"?"#ff6b6b":s.state==="degraded"?"#ffbb66":accent
 radius: 18
 color: Qt.rgba(.035,.065,.085,glass)
 border.color: Qt.rgba(accent.r,accent.g,accent.b,.20)
 border.width: 1
 clip: true
 function fmt(v,p){return v===undefined?"—":Number(v).toLocaleString(Qt.locale(),'f',p)}
 Rectangle {anchors.fill:parent;radius:parent.radius;gradient:Gradient{GradientStop{position:0;color:"#15bce4ed"}GradientStop{position:.48;color:"#027cabb5"}GradientStop{position:1;color:"#0a000000"}}}
 Rectangle {x:22;y:0;width:parent.width-44;height:1;color:Qt.rgba(.8,1,1,.25)}
 Rectangle {x:22;y:0;width:42;height:2;color:panel.stateColor}
 RowLayout {
  anchors.fill:parent;anchors.leftMargin:22;anchors.rightMargin:22;anchors.topMargin:18;anchors.bottomMargin:16;spacing:22
  // identity + score
  ColumnLayout {spacing:2;Layout.preferredWidth:190;Layout.fillHeight:true
   RowLayout {spacing:8
    Text {text:"Π";color:panel.accent;font.family:"monospace";font.pixelSize:12}
    Text {text:"JFP PARADOX";color:"#b4ced5";font.pixelSize:13;font.letterSpacing:1}
    Rectangle {width:5;height:5;radius:3;color:panel.stateColor}
   }
   Text {text:qsTr("Zarządzanie siecią · dry-run");color:"#7898a5";font.pixelSize:10;font.letterSpacing:.6}
   Item {Layout.fillHeight:true}
   RowLayout {spacing:6
    Text {text:panel.ready?panel.fmt(panel.s.score,0):"—";font.pixelSize:40;font.weight:Font.Light;font.letterSpacing:-2;color:"#effaff"}
    Text {text:"/100";font.pixelSize:13;color:panel.accent;Layout.alignment:Qt.AlignBottom;Layout.bottomMargin:8}
   }
  }
  // status + metrics
  ColumnLayout {spacing:7;Layout.fillWidth:true;Layout.fillHeight:true
   RowLayout {spacing:10
    Rectangle {implicitWidth:stateText.implicitWidth+22;implicitHeight:22;radius:11;color:Qt.rgba(panel.stateColor.r,panel.stateColor.g,panel.stateColor.b,.12);border.color:Qt.rgba(panel.stateColor.r,panel.stateColor.g,panel.stateColor.b,.45)
     Text {id:stateText;anchors.centerIn:parent;color:panel.stateColor;font.pixelSize:10;font.letterSpacing:1
      text:!panel.ready?qsTr("POMIAR…"):panel.s.state==="critical"?qsTr("KRYTYCZNY"):panel.s.state==="degraded"?qsTr("POGORSZONY"):qsTr("ZDROWY")}
    }
    Text {Layout.fillWidth:true;elide:Text.ElideRight;color:"#8ca6b0";font.pixelSize:11;textFormat:Text.PlainText
     text:!panel.ready?qsTr("Sonda → %1").arg(panel.s.target||""):(panel.s.breached&&panel.s.breached.length?qsTr("Progi: %1").arg(panel.s.breached.join(", ")):qsTr("Wszystkie progi Governora w normie"))}
   }
   GridLayout {columns:4;columnSpacing:22;rowSpacing:0;Layout.fillWidth:true
    Repeater {model:[
      {k:qsTr("OPÓŹNIENIE"),v:panel.fmt(panel.s.latency,1),u:"ms"},
      {k:qsTr("UTRATA"),v:panel.fmt(panel.s.loss,1),u:"%"},
      {k:qsTr("JITTER"),v:panel.fmt(panel.s.jitter,1),u:"ms"},
      {k:qsTr("PASMO"),v:panel.s.linkKnown?panel.fmt(panel.s.bandwidth,0):"—",u:panel.s.linkKnown?"%":""}]
     ColumnLayout {required property var modelData;spacing:0
      Text {text:modelData.k;font.family:"monospace";font.pixelSize:9;font.letterSpacing:1;color:"#52737e"}
      RowLayout {spacing:3
       Text {text:panel.ready?modelData.v:"—";font.pixelSize:20;font.weight:Font.Light;color:"#effaff"}
       Text {text:panel.ready?modelData.u:"";font.pixelSize:10;color:panel.accent;Layout.alignment:Qt.AlignBottom;Layout.bottomMargin:3}
      }
     }
    }
   }
   Text {Layout.fillWidth:true;elide:Text.ElideRight;font.pixelSize:11;textFormat:Text.PlainText
    color:panel.s.action?(panel.s.armed?"#ff9b8a":"#ffcf8a"):"#6f8f99"
    text:!panel.ready?"":panel.s.action?qsTr("Supervisor %1 → %2 · %3 · nie wykonano (dry-run)").arg(panel.s.level).arg(panel.s.action).arg(panel.s.armed?qsTr("streak %1/3 — uzbrojone").arg(panel.s.streak):qsTr("streak %1/3").arg(panel.s.streak)):qsTr("Supervisor: brak rekomendacji · g(s,hₙ₊₁) = %1").arg(panel.fmt(panel.s.gradient,2))}
  }
  // health history
  ColumnLayout {spacing:4;Layout.preferredWidth:Math.min(340,panel.width*.3);Layout.fillHeight:true;visible:panel.width>760
   Canvas {
    id:graph;Layout.fillWidth:true;Layout.fillHeight:true
    property var samples:panel.s.history||[]
    onSamplesChanged:requestPaint();onWidthChanged:requestPaint();onHeightChanged:requestPaint()
    onPaint:{
     var c=getContext("2d");c.reset();c.strokeStyle="#143641";c.lineWidth=1;
     for(var j=0;j<3;j++){var y=4+j*(height-8)/2;c.beginPath();c.moveTo(0,y);c.lineTo(width,y);c.stroke();}
     for(var k=0;k<7;k++){var x=k*width/6;c.beginPath();c.moveTo(x,0);c.lineTo(x,height);c.stroke();}
     if(samples.length<2)return;
     var step=width/89,first=width-(samples.length-1)*step;
     function yy(v){return height-4-Math.min(1,Math.max(0,v/100))*(height-8);}
     c.beginPath();c.moveTo(first,height);for(var i=0;i<samples.length;i++)c.lineTo(first+i*step,yy(samples[i]));c.lineTo(width,height);c.closePath();
     var g=c.createLinearGradient(0,0,0,height);g.addColorStop(0,Qt.rgba(panel.accent.r,panel.accent.g,panel.accent.b,.28));g.addColorStop(1,"#0031f7a5");c.fillStyle=g;c.fill();
     c.beginPath();for(var n=0;n<samples.length;n++){var px=first+n*step,py=yy(samples[n]);if(n===0)c.moveTo(px,py);else c.lineTo(px,py);}c.strokeStyle=panel.accent;c.lineWidth=1.8;c.stroke();
    }
   }
   RowLayout {Layout.fillWidth:true
    Text {text:qsTr("ZDROWIE SIECI / 7,5 min");font.family:"monospace";font.pixelSize:9;font.letterSpacing:1;color:"#52737e"}
    Item {Layout.fillWidth:true}
    Text {text:qsTr("PROOF #%1").arg(panel.s.entries||0);font.family:"monospace";font.pixelSize:9;color:panel.accent
     ToolTip.text:qsTr("Łańcuch SHA-256: %1").arg(panel.s.proof||"");ToolTip.visible:proofHover.hovered;HoverHandler{id:proofHover}}
   }
  }
 }
}
