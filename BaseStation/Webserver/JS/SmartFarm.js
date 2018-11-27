function graphicIDELoad()
{
    loadXMLBoards();
}

function loadXMLBoards()
{
    var rawFile = new XMLHttpRequest();
    rawFile.open("GET", "./Boards.xml", true);
    rawFile.onreadystatechange = function()
    {
        if(rawFile.readyState === 4)
        {
            if(rawFile.status === 200 || rawFile.status === 0)
            {
                var allText = rawFile.responseText;
                parseXML(allText);
            }
        }
    }
    rawFile.send(null);
}

function parseXML(text)
{
    var parser = new DOMParser();
    var xmlDoc = parser.parseFromString(text,"text/xml");

    var x = xmlDoc.getElementsByTagName("Sensor");

    for(var i = 0;i < x.length;i++)
    {
        var port = x[i].getAttribute("port");
        var sensor = x[i].childNodes[0].nodeValue;

        if(port === "portD")
        {
            for(var j = 0;j < 8;j++)
            {

                var option = document.createElement("option");
                option.setAttribute("value",sensor);
                var text = document.createTextNode(sensor);
                option.appendChild(text);

                var temp = port + (j + 1).toString();
                var select = document.getElementById(temp);
                select.appendChild(option);
            }
        }
        else
        {

            var option = document.createElement("option");
            option.setAttribute("value",sensor);
            var text = document.createTextNode(sensor);
            option.appendChild(text);

            var select = document.getElementById(port);
            select.appendChild(option);
        }

    }
}

function sendDataToServer()
{
    var board = extractGraphicData();
    var program = parseData(board);
    fileSave(program);
}

function saveDataForEditor()
{
    var boardString = JSON.stringify(extractGraphicData());
    console.log(boardString);
    localStorage.setItem("SmartFarmBoard", boardString);
    window.open("text_ide.html");
}

function extractGraphicData()
{
    var board = new Object();
    board.SDCard = document.getElementById("SDCard").value;
    board.portA = document.getElementById("portA").value;
    board.portB = document.getElementById("portB").value;
    board.portC = document.getElementById("portC").value;
    board.portD1 = document.getElementById("portD1").value;
    board.portD2 = document.getElementById("portD2").value;
    board.portD3 = document.getElementById("portD3").value;
    board.portD4 = document.getElementById("portD4").value;
    board.portD5 = document.getElementById("portD5").value;
    board.portD6 = document.getElementById("portD6").value;
    board.portD7 = document.getElementById("portD7").value;
    board.portD8 = document.getElementById("portD8").value;
    board.portE = document.getElementById("portE").value;
    board.portF = document.getElementById("portF").value;

    return(board);
}

function editorFileSave()
{
    fileSave(ace.edit("editor").getValue());
}

function fileSave(program)
{
    $.ajax( {   type: 'POST',
                data: {'program':program },
                url: './PHP/fileSave.php',
                success: function ( data ) {
                    alert( data );
                },
                error: function ( xhr ) {
                    alert("Error");
                }
            });
}

function textIDELoad()
{
    createAceEditor();
    loadDataForEditor();
}

function createAceEditor()
{
    var fontSize = 16;
    var editor = ace.edit("editor");
    editor.setTheme("ace/theme/terminal");
    editor.session.setMode("ace/mode/c_cpp");
    document.getElementById("editor").style.fontSize=fontSize.toString() + "px";
    editor.resize(true);
}

function loadDataForEditor()
{
    var board = JSON.parse(localStorage.getItem("SmartFarmBoard"));

    var text = parseData(board);

    ace.edit("editor").gotoLine(12);
    ace.edit("editor").insert(text);
}

function parseData(board)
{
    var timestamp = 'BoardID + " TS " + smf.timeStamp()';
    var volts = 'BoardID + " V " + smf.readVolts()';
    var portA = loadPortA(board.portA);
    var portB = loadPortB(board.portB);
    var portC = loadPortC(board.portC);
    var portD1 = loadPortD1(board.portD1);
    var portD2 = loadPortD2(board.portD2);
    var portD3 = loadPortD3(board.portD3);
    var portD4 = loadPortD4(board.portD4);
    var portD5 = loadPortD5(board.portD5);
    var portD6 = loadPortD6(board.portD6);
    var portD7 = loadPortD7(board.portD7);
    var portD8 = loadPortD8(board.portD8);
    var portE = loadPortE(board.portE);
    var portF = loadPortE(board.portF);
    var SDCard = board.SDCard;

    var ports = [volts, portA, portB, portC, portD1, portD2, portD3, portD4, portD5, portD6, portD7, portD8, portE, portF];

    var editorText = '#include <SmartFarmMeasureDAQv6_2.h>\n\nSmartFarmMeasure smf;\n\nvoid setup()\n{\n    String BoardID = "A";\n\n    smf.finishUp();\n    Serial.begin(57600);\n    Wire.begin();\n\n    smf.setupAll();\n\n';

    for(var i = 0;i < ports.length;i++)
    {
        if(ports[i] != null)
        {
            var serialPrint = "    Serial.println(" + ports[i] + ");\n";
            editorText += serialPrint;
        }
    }

    editorText += "\n    Serial.flush();\n    Serial.end();\n    delay(2000);\n\n";

    if(SDCard === "MicroSD")
    {

        var serialPrint = "    smf.write2SD(" + timestamp + ");\n";
        editorText += serialPrint;

        for(var i = 0;i < ports.length;i++)
        {
            if(ports[i] != null)
            {
                var serialPrint = "    smf.write2SD(" + ports[i] + ");\n";
                editorText += serialPrint;
            }
        }
    }

    editorText += "}\n\nvoid loop()\n{\n    //Nothing to loop through\n}\n";

    return(editorText);
}

function loadPortA(sensor)
{
    var portA = 'BoardID + " A " + ';

    if(sensor === "SDI12 (Decagon)" || sensor === "SDI12 (Rugged Decagon)")
    {
        portA += "smf.readDecSensors()";
        return(portA);
    }

    return(null);
}

function loadPortB(sensor)
{
    var portB = 'BoardID + " B " + ';

    /*if(sensor === )
    {
        portB += "smf.function()";
        return(portB);
    }*/

    return(null);
}

function loadPortC(sensor)
{
    var portC = 'BoardID + " C " + ';

    /*if(sensor === )
    {
        portC += "smf.function()";
        return(portC);
    }*/

    return(null);
}

function loadPortD1(sensor)
{
    var portD1 = 'BoardID + " D1 " + ';

    if(sensor === "Watermark Resistor")
    {
        portD1 += "smf.readWM(1)";
        return(portD1);
    }

    return(null);
}

function loadPortD2(sensor)
{
    var portD2 = 'BoardID + " D2 " + ';

    if(sensor === "Watermark Resistor")
    {
        portD2 += "smf.readWM(2)";
        return(portD2);
    }

    return(null);
}

function loadPortD3(sensor)
{
    var portD3 = 'BoardID + " D3 " + ';

    if(sensor === "Watermark Resistor")
    {
        portD3 += "smf.readWM(3)";
        return(portD3);
    }

    return(null);
}

function loadPortD4(sensor)
{
    var portD4 = 'BoardID + " D4 " + ';

    if(sensor === "Watermark Resistor")
    {
        portD4 += "smf.readWM(4)";
        return(portD4);
    }

    return(null);
}

function loadPortD5(sensor)
{
    var portD5 = 'BoardID + " D5 " + ';

    if(sensor === "Watermark Resistor")
    {
        portD5 += "smf.readWM(5)";
        return(portD5);
    }

    return(null);
}

function loadPortD6(sensor)
{
    var portD6 = 'BoardID + " D6 " + ';

    if(sensor === "Watermark Resistor")
    {
        portD6 += "smf.readWM(6)";
        return(portD6);
    }

    return(null);
}

function loadPortD7(sensor)
{
    var portD7 = 'BoardID + " D7 " + ';

    if(sensor === "Watermark Resistor")
    {
        portD7 += "smf.readWM(7)";
        return(portD7);
    }

    return(null);
}

function loadPortD8(sensor)
{
    var portD8 = 'BoardID + " D8 " + ';

    if(sensor === "Watermark Resistor")
    {
        portD8 += "smf.readWM(8)";
        return(portD8);
    }

    return(null);
}

function loadPortE(sensor)
{
    var portE = 'BoardID + " E " + ';

    if(sensor === "Temperature")
    {
        portE += "smf.readTemps()";
        return(portE);
    }

    return(null);
}

function loadPortF(sensor)
{
    var portF = 'BoardID + " F " + ';

    /*if(sensor === )
    {
        portF += "smf.function()";
        return(portF);
    }*/

    return(null);
}
