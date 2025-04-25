document.addEventListener('DOMContentLoaded', function() {
  initColorWheel();
  initLEDToggles();
  initRGBToggle();
});

let colorPicker;

function sendRGBColorCommand(rgb) {
  console.log("Sending RGB color:", rgb);

  fetch('/api/rgb', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify({
      r: rgb.r,
      g: rgb.g,
      b: rgb.b
    })
  })
  .then(response => response.json())
  .then(data => console.log('Success:', data))
  .catch((error) => console.error('Error:', error));
}

function initColorWheel() {
  const colorWheelElement = document.getElementById('colorWheel');
  if (!colorWheelElement) return;

  colorPicker = new iro.ColorPicker("#colorWheel", {
    width: 150,
    color: "#ff0000",
    borderWidth: 2,
    borderColor: "#fff",
  });

  colorPicker.on("color:change", function(color) {
    document.querySelector("input[name='r']").value = color.rgb.r;
    document.querySelector("input[name='g']").value = color.rgb.g;
    document.querySelector("input[name='b']").value = color.rgb.b;

    const rgbToggle = document.querySelector('.rgb-toggle');
    if (rgbToggle && rgbToggle.classList.contains('active')) {
      updateRGBButtonColor(color.rgb);
      sendRGBColorCommand(color.rgb);
    }
  });

  const rgbInputs = document.querySelectorAll('.rgb-values input');
  rgbInputs.forEach(input => {
    input.addEventListener('change', function() {
      const r = parseInt(document.querySelector("input[name='r']").value);
      const g = parseInt(document.querySelector("input[name='g']").value);
      const b = parseInt(document.querySelector("input[name='b']").value);
      const rgb = { r, g, b };

      colorPicker.color.rgb = rgb;

      const rgbToggle = document.querySelector('.rgb-toggle');
      if (rgbToggle && rgbToggle.classList.contains('active')) {
        sendRGBColorCommand(rgb);
      }
    });
  });
}

function initLEDToggles() {
  const ledButtons = document.querySelectorAll('.led-toggle');

  ledButtons.forEach(button => {
    button.addEventListener('click', function() {
      this.classList.toggle('active');

      const isRed = this.classList.contains('red');
      const isGreen = this.classList.contains('green');
      const isBlue = this.classList.contains('blue');
      const isYellow = this.classList.contains('yellow');

      const isActive = this.classList.contains('active');

      if (isRed) sendLEDCommand('red', isActive);
      if (isGreen) sendLEDCommand('green', isActive);
      if (isBlue) sendLEDCommand('blue', isActive);
      if (isYellow) sendLEDCommand('yellow', isActive);

      const rgbToggle = document.querySelector('.rgb-toggle');
      if (rgbToggle && rgbToggle.classList.contains('active')) {
        rgbToggle.classList.remove('active');
        sendRGBToggleCommand(false);
        resetRGBButtonStyle();
      }
    });
  });
}

function initRGBToggle() {
  const rgbToggle = document.querySelector('.rgb-toggle');
  if (!rgbToggle) return;

  rgbToggle.addEventListener('click', function() {
    this.classList.toggle('active');
    const isActive = this.classList.contains('active');

    sendRGBToggleCommand(isActive);

    if (isActive) {
      const r = parseInt(document.querySelector("input[name='r']").value);
      const g = parseInt(document.querySelector("input[name='g']").value);
      const b = parseInt(document.querySelector("input[name='b']").value);
      const rgb = { r, g, b };

      updateRGBButtonColor(rgb);
      sendRGBColorCommand(rgb);
    } else {
      resetRGBButtonStyle();
    }
  });
}

function updateRGBButtonColor(rgb) {
  const rgbToggle = document.querySelector('.rgb-toggle');
  if (!rgbToggle) return;

  console.log("Updating button to color:", rgb);

  const brightness = (rgb.r * 299 + rgb.g * 587 + rgb.b * 114) / 1000;
  const textColor = brightness < 125 ? 'white' : 'black';

  rgbToggle.style.setProperty('background-color', `rgb(${rgb.r}, ${rgb.g}, ${rgb.b})`, 'important');
  rgbToggle.style.setProperty('color', textColor, 'important');
  rgbToggle.style.setProperty('border', 'none', 'important');
}

function resetRGBButtonStyle() {
  const rgbToggle = document.querySelector('.rgb-toggle');
  if (!rgbToggle) return;

  rgbToggle.removeAttribute('style');
}

function sendLEDCommand(color, isOn) {
  console.log(`LED ${color} set to ${isOn ? 'ON' : 'OFF'}`);
}

function sendRGBToggleCommand(isActive) {
  console.log(`RGB mode set to ${isActive ? 'ON' : 'OFF'}`);
}
