/**
WebPage
@author Vadim Coselev
@author Asier Legaz
*/


String index_html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>prueba_enviar_recibir</title>
</head>

<body>
    <p>TAREA A REALIZAR</p>

    <!-- Menú seleccionable para accion -->
    <select id="Accion">
        <option value="1">IDLE</option>
        <option value="2">VUELTA DE RECONOCIMIENTO</option>
        <option value="3">RECOGIDA PALET</option>
        <option value="4">SERIAL WIFI</option>
    </select>
  <input type="submit" value="Enviar tarea" onClick="sendRequest('Accion')"><br><br>

  <!-- Entrada manual para Nodo actual -->
  <p>Nodo actual</p>
  <select id="Nodo_actual">
    <option value="0">0</option>
    <option value="1">1</option>
    <option value="2">2</option>
    <option value="3">3</option>
    <option value="4">4</option>
    <option value="5">5</option>
    <option value="6">6</option>
    <option value="7">7</option>
    <option value="8">8</option>
    <option value="9">9</option>
    <option value="10">10</option>
    <option value="11">11</option>
    <option value="12">12</option>
    <option value="13">13</option>
    <option value="14">14</option>
    <option value="15">15</option>
    <option value="16">16</option>
    <option value="17">17</option>
    <option value="18">18</option>
    <option value="19">19</option>
    <option value="20">20</option>
    <option value="21">21</option>
  </select>
  <input type="submit" value="Enviar nodo actual" onClick="sendRequest('Nodo_actual')"><br><br>

        <!-- Menú seleccionable para Orientacion actual -->
    <p>Orientación actual</p>
    <select id="Orientacion_actual">
        <option value="0">0º</option>
        <option value="90">90º</option>
        <option value="180">180º</option>
        <option value="270">270º</option>
    </select>
    <input type="submit" value="Enviar orientación actual " onClick="sendRequest('Orientacion_actual')">
    <br><br>

   <!-- Menú seleccionable para Entrega azul -->
    <p>Entrega palet azul</p>
    <select id="Entrega_azul">
        <option value="7">ENTREGA 1</option>
        <option value="8">ENTREGA 2</option>
        <option value="9">ENTREGA 3</option>
        <option value="16">ENTREGA 4</option>
    </select><br>

   <!-- Menú seleccionable para Entrega rojo -->
    <p>Entrega palet rojo</p>
    <select id="Entrega_rojo">
        <option value="7">ENTREGA 1</option>
        <option value="8">ENTREGA 2</option>
        <option value="9">ENTREGA 3</option>
        <option value="16">ENTREGA 4</option>
    </select><br>

     <!-- Menú seleccionable para Entrega verde -->
    <p>Entrega palet verde</p>
    <select id="Entrega_verde">
        <option value="7">ENTREGA 1</option>
        <option value="8">ENTREGA 2</option>
        <option value="9">ENTREGA 3</option>
        <option value="16">ENTREGA 4</option>
    </select><br><br><br>

    <!-- Botones separados para enviar cada valor por separado -->
    <input type="submit" value="Enviar orden de entrega" onClick="sendRequests(['Entrega_azul', 'Entrega_rojo', 'Entrega_verde'])">

<!-- Div for showing received text -->
<div id="outputText">Texto predeterminado</div>

<!-- JavaScript code -->
<script>
    var socket = new WebSocket('ws://' + window.location.hostname + '/');  // Port 80 is used by default

    socket.onopen = function (event) {
        console.log("WebSocket connection opened.");
    };

    socket.onmessage = function (event) {
        var data = JSON.parse(event.data);
        if (data.outputText) {
            document.getElementById('outputText').innerHTML = data.outputText;
        }
        console.log("Received WebSocket message:", data);
    };

    socket.onclose = function (event) {
        console.log("WebSocket connection closed.");
    };

    socket.onerror = function (event) {
        console.error("WebSocket error:", event);
    };

    function sendRequests(cmdArray) {
        for (var i = 0; i < cmdArray.length; i++) {
            sendRequest(cmdArray[i]);
            delay(1000);
        }

    }

    function sendRequest(cmd) {
        var selectedValue = document.getElementById(cmd).options[document.getElementById(cmd).selectedIndex].value;

        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/cmd?" + cmd + "=" + selectedValue, true);
        xhr.send();
    }
  function delay(time){
	return new Promise(resolve => setTimeout(resolve, time));
}
</script>

</body>
</html>

)rawliteral";
