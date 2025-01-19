var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
    websocket.send("states");
}
  
function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
} 

function onMessage(event) {
    var myObj = JSON.parse(event.data);
            console.log(myObj);
            for (i in myObj.gpios){
                var output = myObj.gpios[i].output;
                var state = myObj.gpios[i].state;
                console.log(output);
                console.log(state);
                if (state == "1"){
                    document.getElementById(output).checked = true;
                    document.getElementById(output+"s").innerHTML = "ON";
                }
                else{
                    document.getElementById(output).checked = false;
                    document.getElementById(output+"s").innerHTML = "OFF";
                }
            }
    if (myObj.T_dry !== undefined) {
        document.getElementById("T_dry").textContent = myObj.T_dry + " C";
    }
    if (myObj.T_wet !== undefined) {
        document.getElementById("T_wet").textContent =  myObj.T_wet + " C";
    }
    if (myObj.RH !== undefined) {
        document.getElementById("RH").textContent =  myObj.RH + " %";
    }
    if (myObj.rhSetpoint !== undefined) {
        document.getElementById("rhSetpoint").textContent =  myObj.rhSetpoint + " %";
    }
    if (myObj.relayState !== undefined) {
        var relayStateElement = document.getElementById("relay-state");
        if (myObj.relayState === "1") {
           relayStateElement.textContent = "ON";
        } else {
           relayStateElement.textContent = "OFF";
        }
     }     
    console.log(event.data);
}

// Send Requests to Control GPIOs
function toggleCheckbox (element) {
    console.log(element.id);
    websocket.send(element.id);
    if (element.checked){
        document.getElementById(element.id+"s").innerHTML = "ON";
    }
    else {
        document.getElementById(element.id+"s").innerHTML = "OFF"; 
    }
}

function BPR1(){
    websocket.send("BR1");
}
function BPY1(){
    websocket.send("BY1");
}
function BPY2(){
    websocket.send("BY2");
}
function BPG1(){
    websocket.send("BG1");
}