// Arreglos para almacenar el historial de lecturas
let historyGalga = [];
let historySonico = [];

// UPDATE DATA - SENSOR 1 (GALGA)
function updateData() {
  fetch('/data')
    .then(r => r.json())
    .then(data => {
      document.getElementById('peso').innerHTML = data.peso.toFixed(1) + ' g';
      document.getElementById('nivel').innerHTML = data.nivel.toFixed(2) + ' cm';
      document.getElementById('porcentaje').innerHTML = data.porcentaje.toFixed(1) + ' %';
      document.getElementById('radio').innerHTML = data.radio.toFixed(1) + ' cm';
      document.getElementById('altura').innerHTML = data.altura.toFixed(1) + ' cm';
      document.getElementById('timestamp').innerHTML = data.timestamp;

      updateTank(data.porcentaje, 'water', 'waterText');
      checkAlerts(data.porcentaje);

      // Almacenar registro en el historial para el CSV
      historyGalga.push({
        time: data.timestamp,
        peso: data.peso.toFixed(1),
        nivel: data.nivel.toFixed(2),
        pct: data.porcentaje.toFixed(1)
      });

      document.getElementById('status').className = 'status connected';
      document.getElementById('status').innerHTML = '<div class="dot"></div> Galga: Conectado';
    })
    .catch(e => {
      document.getElementById('status').className = 'status disconnected';
      document.getElementById('status').innerHTML = '<div class="dot"></div> Galga: Desconectado';
    });
}


// UPDATE DATA - SENSOR 2 (ULTRASÓNICO)
function updateData2() {
  fetch('/data2')
    .then(r => r.json())
    .then(data => {
      document.getElementById('distancia').innerHTML = data.distancia.toFixed(1) + ' cm';
      document.getElementById('nivel2').innerHTML = data.nivel.toFixed(2) + ' cm';
      document.getElementById('porcentaje2').innerHTML = data.porcentaje.toFixed(1) + ' %';
      document.getElementById('alturaRef').innerHTML = data.alturaRef.toFixed(1) + ' cm';
      document.getElementById('timestamp2').innerHTML = data.timestamp;

      updateTank(data.porcentaje, 'water2', 'waterText2');

      // Almacenar registro en el historial para el CSV
      historySonico.push({
        time: data.timestamp,
        dist: data.distancia.toFixed(1),
        nivel: data.nivel.toFixed(2),
        pct: data.porcentaje.toFixed(1)
      });

      document.getElementById('status2').className = 'status connected';
      document.getElementById('status2').innerHTML = '<div class="dot"></div> Sonido: Conectado';
    })
    .catch(e => {
      document.getElementById('status2').className = 'status disconnected';
      document.getElementById('status2').innerHTML = '<div class="dot"></div> Sonido: Desconectado';
    });
}


// COMPONENTES VISUALES
function updateTank(pct, waterId, textId) {
  const water = document.getElementById(waterId);
  const text = document.getElementById(textId);
  
  let safePct = Math.max(0, Math.min(100, pct));
  water.style.height = safePct + '%';
  text.innerHTML = safePct.toFixed(1) + '%';
}

function checkAlerts(pct) {
  const alert = document.getElementById('alertBox');
  if(pct < 15) {
    alert.style.display = 'block';
    alert.innerHTML = 'Alerta: Nivel critico bajo (menor al 15%)';
  } else if(pct > 90) {
    alert.style.display = 'block';
    alert.innerHTML = 'Aviso: Tanque casi lleno (mayor al 90%)';
  } else {
    alert.style.display = 'none';
  }
}


// COMANDOS Y MODALES - SENSOR 1 (GALGA)

function sendTare() {
  fetch('/tare', { method: 'POST' });
  console.log('Tare enviado');
}

function openModal() {
  document.getElementById('newRadio').value = parseFloat(document.getElementById('radio').innerHTML) || 0;
  document.getElementById('newAltura').value = parseFloat(document.getElementById('altura').innerHTML) || 0;
  document.getElementById('modal').classList.add('active');
}

function closeModal() {
  document.getElementById('modal').classList.remove('active');
}

function saveDimensions() {
  const radio = parseFloat(document.getElementById('newRadio').value);
  const altura = parseFloat(document.getElementById('newAltura').value);

  if(radio > 0 && altura > 0) {
    fetch('/setDim?radio=' + radio + '&altura=' + altura);
    closeModal();
    console.log('Dimensiones de la galga guardadas');
  } else {
    alert('Valores invalidos');
  }
}

// Exportacion a CSV 
function downloadCSV() {
  if(historyGalga.length === 0) {
    alert("No hay datos acumulados todavía.");
    return;
  }
  
  let csvContent = "sep=;\n";
  csvContent += "Fecha y Hora;Peso (g);Nivel (cm);Porcentaje (%)\n";
  
  historyGalga.forEach(row => {
    csvContent += `${row.time};${row.peso};${row.nivel};${row.pct}\n`;
  });

  const blob = new Blob([new Uint8Array([0xEF, 0xBB, 0xBF]), csvContent], { type: "text/csv;charset=utf-8;" });
  const url = URL.createObjectURL(blob);
  
  const link = document.createElement("a");
  link.setAttribute("href", url);
  link.setAttribute("download", "Historial_Galga.csv");
  document.body.appendChild(link);
  link.click();
  document.body.removeChild(link);
  URL.revokeObjectURL(url);

  // Vaciar el historial de forma automática después de la descarga exitosa
  historyGalga = [];
  console.log("Historial de galga reiniciado automáticamente.");
}

// Funcion para borrar los datos de la galga por medio de boton
function limpiarHistorialGalga() {
  if(historyGalga.length === 0) {
    alert("El historial de la Galga ya se encuentra vacío.");
    return;
  }
  historyGalga = [];
  alert("Historial de Galga borrado correctamente.");
}


// COMANDOS Y MODALES - SENSOR 2 (ULTRASÓNICO)
function openModal2() {
  document.getElementById('newAlturaRef').value = parseFloat(document.getElementById('alturaRef').innerHTML) || 0;
  document.getElementById('modal2').classList.add('active');
}

function closeModal2() {
  document.getElementById('modal2').classList.remove('active');
}

// Exportación del historial del Ultrasonico con corrección multicolumna para Excel
function downloadCSV2() {
  if(historySonico.length === 0) {
    alert("No hay datos acumulados todavía.");
    return;
  }
  
  let csvContent = "sep=;\n";
  csvContent += "Fecha y Hora;Distancia (cm);Nivel (cm);Porcentaje (%)\n";
  
  historySonico.forEach(row => {
    csvContent += `${row.time};${row.dist};${row.nivel};${row.pct}\n`;
  });

  const blob = new Blob([new Uint8Array([0xEF, 0xBB, 0xBF]), csvContent], { type: "text/csv;charset=utf-8;" });
  const url = URL.createObjectURL(blob);
  
  const link = document.createElement("a");
  link.setAttribute("href", url);
  link.setAttribute("download", "Historial_Ultrasonico.csv");
  document.body.appendChild(link);
  link.click();
  document.body.removeChild(link);
  URL.revokeObjectURL(url);

  // Vaciar el historial de forma automática después de la descarga exitosa
  historySonico = [];
  console.log("Historial ultrasónico reiniciado automáticamente.");
}

function saveDimensions2() {
  const alturaRef = parseFloat(document.getElementById('newAlturaRef').value);

  if(alturaRef > 0) {
    fetch('/setDim2?alturaRef=' + alturaRef);
    closeModal2();
    console.log('Altura de referencia ultrasonica guardada');
  } else {
    alert('Valor invalido');
  }
}

// Funcion para borrar los datos del ultrasonido por medio de boton
function limpiarHistorialSonico() {
  if(historySonico.length === 0) {
    alert("El historial del Ultrasonido ya se encuentra vacío.");
    return;
  }
  historySonico = [];
  alert("Historial Ultrasonico borrado correctamente.");
}

// MANEJO DE TECLAS Y ARRANQUE GENERAL
document.addEventListener('keydown', (e) => {
  if(e.key === 'Escape') {
    closeModal();
    closeModal2();
  }
});

function tick() {
  updateData();
  updateData2();
}

tick();
setInterval(tick, 1000);