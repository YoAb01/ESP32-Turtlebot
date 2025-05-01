document.addEventListener('DOMContentLoaded', function() {
  startSensorDataPulling();
});

function startSensorDataPulling(){
  setInterval(fetchSensorData, 1000);
}

function fetchSensorData() {
  fetch('/api/sensors')
    .then(response => {
      if (!response.ok) {
        throw new Error('Network response was not ok');
      }
      return response.json();
    })
    .then(data=> {
      updateSensorDisplay(data);
    })
    .catch(error => {
      console.error('Error fetching sensor data: ', error);
    });
}

function updateSensorDisplay(data) {
  // Update Gyro
  if (data.gyro) {
    document.getElementById('gyro-x').textContent = `X: ${data.gyro.x.toFixed(2)} rad/s`;
    document.getElementById('gyro-y').textContent = `Y: ${data.gyro.y.toFixed(2)} rad/s`;
    document.getElementById('gyro-z').textContent = `Z: ${data.gyro.z.toFixed(2)} rad/s`;
  }

  // Update Accel
  if (data.accel) {
    document.getElementById('accel-x').textContent = `X: ${data.accel.x.toFixed(2)} m/s²`;
    document.getElementById('accel-y').textContent = `Y: ${data.accel.y.toFixed(2)} m/s²`;
    document.getElementById('accel-z').textContent = `Z: ${data.accel.z.toFixed(2)} m/s²`;
  }

  // Update Baro
  if (data.baro) {
    document.getElementById('baro-pressure').textContent = `Pressure: ${data.baro.pressure.toFixed(2)} hPa`;
    document.getElementById('baro-altitude').textContent = `Altitude: ${data.baro.altitude.toFixed(2)} m`;
  }

  // Update Mag
  if (data.mag) {
    document.getElementById('mag-x').textContent = `X: ${data.mag.x.toFixed(2)} μT`;
    document.getElementById('mag-y').textContent = `Y: ${data.mag.y.toFixed(2)} μT`;
    document.getElementById('mag-z').textContent = `Z: ${data.mag.z.toFixed(2)} μT`;
  }

  // Update gps 
  if (data.gps) {
    document.getElementById('gps-lat').textContent = `Lat: ${data.gps.lat.toFixed(4)}° ${data.gps.lat_dir}`;
    document.getElementById('gps-lon').textContent = `Long: ${data.gps.lon.toFixed(4)}° ${data.gps.lon_dir}`;
    document.getElementById('gps-alt').textContent = `Alt: ${data.gps.alt.toFixed(2)} m`;
    document.getElementById('gps-sats').textContent = `Satellites: ${data.gps.satellites}`;
  }
}
