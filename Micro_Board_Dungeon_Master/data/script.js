var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onload);
var direction;
var actionType;

function onload(event) {
    initWebSocket();
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function submitForm(){
    const rbs = document.querySelectorAll('input[name="direction"]');
    direction;
    for (const rb of rbs) {
        if (rb.checked) {
            direction = rb.value;
            break;
        }
    }
    const ActionTypes = document.querySelectorAll('input[name="Dice"]');
    actionType;
    for (const ActionType of ActionTypes) {
        if (ActionType.checked) {
            actionType = ActionType.value;
            break;
        }
    }

    document.getElementById("motor-state").innerHTML = "motor spinning...";
    document.getElementById("motor-state").style.color = "blue";
    if (direction=="CCW"){
        document.getElementById("gear").classList.add("spin-back");
    }
    else{
        document.getElementById("gear").classList.add("spin");
    }
    
    var steps = document.getElementById("steps").value;
    websocket.send("Steps"+steps+"ActionType"+actionType+"Direction"+direction);
}

function onMessage(event) {
    console.log(event.data);
    messageReturn = event.data;
    if (messageReturn=="stop"){ 
      document.getElementById("motor-state").innerHTML = "DM as finish"
      document.getElementById("motor-state").style.color = "gray";
      document.getElementById("Dice-state").style.color = "gray";
      document.getElementById("gear").classList.remove("spin", "spin-back");
    }
    else if(messageReturn=="CW" || messageReturn=="CCW"){
        document.getElementById("motor-state").innerHTML = "DM spinning...";
        document.getElementById("motor-state").style.color = "blue";
        if (messageReturn=="CCW"){
            document.getElementById("gear").classList.add("spin-back");
        }
        else{
            document.getElementById("gear").classList.add("spin");
        }
    }
    else if(messageReturn!="None" || messageReturn=="Start"){ 
        document.getElementById("Dice-state").innerHTML = messageReturn;
        document.getElementById("Dice-state").style.color = "white";
    }
}