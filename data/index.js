/*

Created by Harry Wieldraaijer on 23 december 2023
Version 3.0 - 23 december 2023
Version 3.1 - 10 januari 2024

*/
var gateway = `ws://${window.location.hostname}/ws` ;
var websocket;
window.addEventListener('load',onLoad);

function onLoad(event){
    initWebSocket();
}

function initWebSocket(){
    console.log('Try to open a websocket connection');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened')
    websocket.send("configuration");
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket,2000);
}

function onMessage(event) {
    var rcvJSON = JSON.parse(event.data);
    console.log(rcvJSON);
    // uncode the rest
}

let settings = {
    settings:{
        block1: {
            startMomentFixed: false,
            startShift: 0,
            startTime: '18:00',
            lightLevel: 80,
            interruptSettings:{
                interruptEnabled: true,
                interruptDuration: 10,
                interruptLightLevel: 100
            }
        },
        block2: {
        startMomentFixed: true,
            startShift: 0,
            startTime: '23:00',
            lightLevel: 60,
            interruptSettings:{
                interruptEnabled: true,
                interruptDuration: 5,
                interruptLightLevel: 80  
            } 
        },
        block3: {
        startMomentFixed: false,
            startShift: -30,
            startTime: '08:00',
            lightLevel: 0,
            interruptSettings:{
                interruptEnabled: false,
                interruptDuration: 0,
                interruptLightLevel: 0  
            } 
        }
    }
    };


if  (localStorage.getItem('storedSetting') !== null) {
        console.log('There are stored settings!');
        let settings=JSON.parse(localStorage.getItem('storedSetting'));
        console.log("The stored values are: ")
        console.log(JSON.stringify(settings));
        showValuesInForm(settings);
} else {
    console.log('There are no stored settings!');
    showValuesInForm(settings);

}

    function block1StartChoice(theStartChoice) {
        if (theStartChoice === 'block1StartTimeFixed') {
            document.querySelector(".block1Fixed").style.visibility = "visible";
            document.querySelector(".block1StartTimeSunset").style.visibility = "hidden";
        } else if (theStartChoice === 'block1StartTimeSunset') {
            document.querySelector(".block1StartTimeSunset").style.visibility = "visible";
            document.querySelector(".block1Fixed").style.visibility = "hidden";
        }
    }   
    function block1MovementInterrupt() {
        var checkBox = document.getElementById("block1MovementInterrupt");
        if (checkBox.checked == true) {
            document.querySelector(".block1MovementInterrupt").style.visibility = "visible";
        } else {
            document.querySelector(".block1MovementInterrupt").style.visibility = "hidden"; 
        }
    }
    function block1StartOffset(selectedValue) {
        console.log(selectedValue);
        document.getElementById("block1StartOffsetSlider").value = selectedValue;
        document.getElementById("block1StartOffsetNumber").value = selectedValue;
    }
    function block1LightLevel(selectedLevel) {
        console.log(selectedLevel);
        document.getElementById("block1LightLevelSlider").value = selectedLevel;
        document.getElementById("block1LightLevelNumber").value = selectedLevel;
    }
    function block1InterruptLightLevel(selectedLevel) {
        console.log(selectedLevel);
        document.getElementById("block1InterruptLightLevelSlider").value = selectedLevel;
        document.getElementById("block1InterruptLightLevelNumber").value = selectedLevel;
    }
    function block1Interruptduration(selectedValue) {
        console.log(selectedValue);
        document.getElementById("block1InterruptdurationSlider").value = selectedValue;
        document.getElementById("block1InterruptdurationNumber").value = selectedValue;
    }

    function block2StartChoice(theStartChoice) {
        if (theStartChoice === 'block2StartTimeFixed') {
            document.querySelector(".block2Fixed").style.visibility = "visible";
            document.querySelector(".block2StartTimeSunset").style.visibility = "hidden";
        } else if (theStartChoice === 'block2StartTimeSunset') {
            document.querySelector(".block2StartTimeSunset").style.visibility = "visible";
            document.querySelector(".block2Fixed").style.visibility = "hidden";
        }
    }   
    function block2MovementInterrupt() {
        var checkBox = document.getElementById("block2MovementInterrupt");
        if (checkBox.checked == true) {
            document.querySelector(".block2MovementInterrupt").style.visibility = "visible";
        } else {
            document.querySelector(".block2MovementInterrupt").style.visibility = "hidden"; 
        }
    }
    function block2StartOffset(selectedValue) {
        console.log(selectedValue);
        document.getElementById("block2StartOffsetSlider").value = selectedValue;
        document.getElementById("block2StartOffsetNumber").value = selectedValue;
    }
    function block2LightLevel(selectedLevel) {
        console.log(selectedLevel);
        document.getElementById("block2LightLevelSlider").value = selectedLevel;
        document.getElementById("block2LightLevelNumber").value = selectedLevel;
    }
    function block2InterruptLightLevel(selectedLevel) {
        console.log(selectedLevel);
        document.getElementById("block2InterruptLightLevelSlider").value = selectedLevel;
        document.getElementById("block2InterruptLightLevelNumber").value = selectedLevel;
    }
    function block2Interruptduration(selectedValue) {
        console.log(selectedValue);
        document.getElementById("block2InterruptdurationSlider").value = selectedValue;
        document.getElementById("block2InterruptdurationNumber").value = selectedValue;
    }
    
    function block3StartChoice(theStartChoice) {
        if (theStartChoice === 'block3StartTimeFixed') {
            document.querySelector(".block3Fixed").style.visibility = "visible";
            document.querySelector(".block3StartTimeSunset").style.visibility = "hidden";
        } else if (theStartChoice === 'block3StartTimeSunset') {
            document.querySelector(".block3StartTimeSunset").style.visibility = "visible";
            document.querySelector(".block3Fixed").style.visibility = "hidden";
        }
    }   
    function block3MovementInterrupt() {
        var checkBox = document.getElementById("block3MovementInterrupt");
        if (checkBox.checked == true) {
            document.querySelector(".block3MovementInterrupt").style.visibility = "visible";
        } else {
            document.querySelector(".block3MovementInterrupt").style.visibility = "hidden"; 
        }
    }
    function block3StartOffset(selectedValue) {
        console.log(selectedValue);
        document.getElementById("block3StartOffsetSlider").value = selectedValue;
        document.getElementById("block3StartOffsetNumber").value = selectedValue;
    }
    function block3LightLevel(selectedLevel) {
        console.log(selectedLevel);
        document.getElementById("block3LightLevelSlider").value = selectedLevel;
        document.getElementById("block3LightLevelNumber").value = selectedLevel;
    }
    function block3InterruptLightLevel(selectedLevel) {
        console.log(selectedLevel);
        document.getElementById("block3InterruptLightLevelSlider").value = selectedLevel;
        document.getElementById("block3InterruptLightLevelNumber").value = selectedLevel;
    }
    function block3Interruptduration(selectedValue) {
        console.log(selectedValue);
        document.getElementById("block3InterruptdurationSlider").value = selectedValue;
        document.getElementById("block3InterruptdurationNumber").value = selectedValue;
    }
    function getCheckboxValue(whichCheckbox){
        var checkBox = document.getElementById(whichCheckbox);
        if (checkBox.checked == true) {
            return true;
        } else {
            return false; 
        }
    }
    function saveButton(){
        settings.settings.block1.startMomentFixed = getCheckboxValue("block1StartTimeFixed");
        settings.settings.block1.startShift = document.getElementById("block1StartOffsetNumber").value;
        settings.settings.block1.startTime = document.getElementById("block1StartTime").value;
        settings.settings.block1.lightLevel = document.getElementById("block1LightLevelNumber").value;
        settings.settings.block1.interruptSettings.interruptEnabled = getCheckboxValue("block1MovementInterrupt");
        settings.settings.block1.interruptSettings.interruptDuration = document.getElementById("block1InterruptdurationNumber").value;
        settings.settings.block1.interruptSettings.interruptLightLevel= document.getElementById("block1InterruptLightLevelNumber").value;
        settings.settings.block2.startMomentFixed = getCheckboxValue("block2StartTimeFixed");
        settings.settings.block2.startShift = document.getElementById("block2StartOffsetNumber").value;
        settings.settings.block2.startTime = document.getElementById("block2StartTime").value;
        settings.settings.block2.lightLevel = document.getElementById("block2LightLevelNumber").value;
        settings.settings.block2.interruptSettings.interruptEnabled = getCheckboxValue("block2MovementInterrupt");
        settings.settings.block2.interruptSettings.interruptDuration = document.getElementById("block2InterruptdurationNumber").value;
        settings.settings.block2.interruptSettings.interruptLightLevel= document.getElementById("block2InterruptLightLevelNumber").value;
        settings.settings.block3.startMomentFixed = getCheckboxValue("block3StartTimeFixed");
        settings.settings.block3.startShift = document.getElementById("block3StartOffsetNumber").value;
        settings.settings.block3.startTime = document.getElementById("block3StartTime").value;
        settings.settings.block3.lightLevel = document.getElementById("block3LightLevelNumber").value;
        settings.settings.block3.interruptSettings.interruptEnabled = getCheckboxValue("block3MovementInterrupt");
        settings.settings.block3.interruptSettings.interruptDuration = document.getElementById("block3InterruptdurationNumber").value;
        settings.settings.block3.interruptSettings.interruptLightLevel= document.getElementById("block3InterruptLightLevelNumber").value;
        console.log(JSON.stringify(settings));
        localStorage.setItem('storedSetting',JSON.stringify(settings));
        websocket.send(JSON.stringify(settings));
    }
    function showValuesInForm(settings){
        // block1
        if (settings.settings.block1.startMomentFixed === true){
            document.getElementById("block1StartTimeFixed").checked = true;
            document.getElementById("block1StartTimeSunset").checked = false;
            block1StartChoice('block1StartTimeFixed');
        } else {
            document.getElementById("block1StartTimeSunset").checked = true;
            document.getElementById("block1StartTimeFixed").checked = false;
            block1StartChoice('block1StartTimeSunset');
        }
        document.getElementById("block1StartTime").value = settings.settings.block1.startTime ;
        document.getElementById("block1LightLevelNumber").value = settings.settings.block1.lightLevel ;
        document.getElementById("block1LightLevelSlider").value = settings.settings.block1.lightLevel ;
        document.getElementById("block1StartOffsetNumber").value = settings.settings.block1.startShift ;
        document.getElementById("block1StartOffsetSlider").value = settings.settings.block1.startShift ;
        if (settings.settings.block1.interruptSettings.interruptEnabled === true){
            document.getElementById("block1MovementInterrupt").checked = true;
            document.querySelector(".block1MovementInterrupt").style.visibility = "visible";
        } else {
            document.getElementById("block1MovementInterrupt").checked = false;
            document.querySelector(".block1MovementInterrupt").style.visibility = "hidden";
        }
        document.getElementById("block1InterruptLightLevelNumber").value = settings.settings.block1.interruptSettings.interruptLightLevel ;
        document.getElementById("block1InterruptLightLevelSlider").value = settings.settings.block1.interruptSettings.interruptLightLevel ;
        document.getElementById("block1InterruptdurationNumber").value = settings.settings.block1.interruptSettings.interruptDuration ;
        document.getElementById("block1InterruptdurationSlider").value = settings.settings.block1.interruptSettings.interruptDuration ;
        // block2
        if (settings.settings.block2.startMomentFixed === true){
            document.getElementById("block2StartTimeFixed").checked = true;
            document.getElementById("block2StartTimeSunset").checked = false;
            block2StartChoice('block2StartTimeFixed');
        } else {
            document.getElementById("block2StartTimeSunset").checked = true;
            document.getElementById("block2StartTimeFixed").checked = false;
            block2StartChoice('block2StartTimeSunset');
        }
        document.getElementById("block2StartTime").value = settings.settings.block2.startTime ;
        document.getElementById("block2LightLevelNumber").value = settings.settings.block2.lightLevel ;
        document.getElementById("block2LightLevelSlider").value = settings.settings.block2.lightLevel ;
        document.getElementById("block2StartOffsetNumber").value = settings.settings.block2.startShift ;
        document.getElementById("block2StartOffsetSlider").value = settings.settings.block2.startShift ;
        if (settings.settings.block2.interruptSettings.interruptEnabled === true){
            document.getElementById("block2MovementInterrupt").checked = true;
            document.querySelector(".block2MovementInterrupt").style.visibility = "visible";
        } else {
            document.getElementById("block2MovementInterrupt").checked = false;
            document.querySelector(".block2MovementInterrupt").style.visibility = "hidden";
        }
        document.getElementById("block2InterruptLightLevelNumber").value = settings.settings.block2.interruptSettings.interruptLightLevel ;
        document.getElementById("block2InterruptLightLevelSlider").value = settings.settings.block2.interruptSettings.interruptLightLevel ;
        document.getElementById("block2InterruptdurationNumber").value = settings.settings.block2.interruptSettings.interruptDuration ;
        document.getElementById("block2InterruptdurationSlider").value = settings.settings.block2.interruptSettings.interruptDuration ;
        // block3
        if (settings.settings.block3.startMomentFixed === true){
            document.getElementById("block3StartTimeFixed").checked = true;
            document.getElementById("block3StartTimeSunset").checked = false;
            block3StartChoice('block3StartTimeFixed');
        } else {
            document.getElementById("block3StartTimeSunset").checked = true;
            document.getElementById("block3StartTimeFixed").checked = false;
            block3StartChoice('block3StartTimeSunset');
        }
        document.getElementById("block3StartTime").value = settings.settings.block3.startTime ;
        document.getElementById("block3LightLevelNumber").value = settings.settings.block3.lightLevel ;
        document.getElementById("block3LightLevelSlider").value = settings.settings.block3.lightLevel ;
        document.getElementById("block3StartOffsetNumber").value = settings.settings.block3.startShift ;
        document.getElementById("block3StartOffsetSlider").value = settings.settings.block3.startShift ;
        if (settings.settings.block3.interruptSettings.interruptEnabled === true){
            document.getElementById("block3MovementInterrupt").checked = true;
            document.querySelector(".block3MovementInterrupt").style.visibility = "visible";
        } else {
            document.getElementById("block3MovementInterrupt").checked = false;
            document.querySelector(".block3MovementInterrupt").style.visibility = "hidden";
        }
        document.getElementById("block3InterruptLightLevelNumber").value = settings.settings.block3.interruptSettings.interruptLightLevel ;
        document.getElementById("block3InterruptLightLevelSlider").value = settings.settings.block3.interruptSettings.interruptLightLevel ;
        document.getElementById("block3InterruptdurationNumber").value = settings.settings.block3.interruptSettings.interruptDuration ;
        document.getElementById("block3InterruptdurationSlider").value = settings.settings.block3.interruptSettings.interruptDuration ;
    }