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
                // console.log(output);
                // console.log(state);
                if (state == "1"){
                    document.getElementById(output).checked = true;
                    document.getElementById(output+"s").innerHTML = "ON";
                }
                else{
                    document.getElementById(output).checked = false;
                    document.getElementById(output+"s").innerHTML = "OFF";
                }
            }
            if (myObj.current != null){
                document.getElementById("current").innerHTML = myObj.current.toFixed(2).toString();
            }else document.getElementById("current").innerHTML = 0;
            if (myObj.power != null){
                document.getElementById("power").innerHTML = myObj.power.toFixed(2).toString();
            }else document.getElementById("power").innerHTML = 0;
            if (myObj.voltage != null){
                document.getElementById("voltage").innerHTML = myObj.voltage.toFixed(2).toString();
            }else document.getElementById("voltage").innerHTML = 0;
            if (myObj.voltage != null){
                document.getElementById("powerfactor").innerHTML = myObj.powerfactor.toFixed(2).toString();
            }else document.getElementById("powerfactor").innerHTML = 0;
            if (myObj.ID != null){
                document.getElementById("ID").innerHTML = myObj.ID.toString();
            }else  document.getElementById("ID").innerHTML = "OSSP";
            if (myObj.APconnected == false){
                document.getElementById("APdisconnected").style.display = "initial";
            }else document.getElementById("APdisconnected").style.display = "none";
            if (myObj.ADCissue == true){
                document.getElementById("ADCissueBanner").style.display = "initial";
            }else document.getElementById("ADCissueBanner").style.display = "none";
            
    // console.log(event.data);
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

function toggleMenu() {
    var menuElement = document.getElementById('menuBody')
    if( menuElement.style.flex == "none" || menuElement.style.flex == "" || menuElement.style.flex == "0 0 auto"){
        menuElement.style.flex = "auto"
        menuElement.classList.remove("mobilehide")
    }else if (menuElement.style.flex == "1 1 auto"){

        menuElement.style.flex = "none"
        menuElement.classList.add("mobilehide")
    }
}