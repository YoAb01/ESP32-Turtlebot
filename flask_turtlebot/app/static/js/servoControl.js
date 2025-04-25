document.addEventListener('DOMContentLoaded', function(){
  initServoControls();
});

function initServoControls(){
  initSliderInputBindings();
  initDirectionButtons();
  initHomeButton();
}

function initSliderInputBindings() {
  const sliderContainers = document.querySelectorAll('.slider-container');
  sliderContainers.forEach(container => {
    const slider = container.querySelector('.servo-slider');
    const input = container.querySelector('.servo-value');
    if(!slider || !input) return;

    // Update the value if the slider changes
    slider.addEventListener('input', function() {
      input.value = this.value;
    });

    // Update the slider if the value changes
    input.addEventListener('change', function() {
      let value = parseInt(this.value);
      value = Math.min(Math.max(value, -90), 90);
      this.value = value;
      slider.value = value;
    });
  });
}

function initDirectionButtons() {
  // Set up direction buttons
  const upButton = document.querySelector('.servo-btn.up');
  const downButton = document.querySelector('.servo-btn.down');
  const leftButton = document.querySelector('.servo-btn.left');
  const rightButton = document.querySelector('.servo-btn.right');

  // Down button
  upButton.addEventListener('click', function() {
    adjustServoValue('vertical', 2);
  });

  // Down button
  downButton.addEventListener('click', function() {
    adjustServoValue('vertical', -2);
  });

  // Left button
  leftButton.addEventListener('click', function() {
    adjustServoValue('horizontal', -2);
  });

  // Right button
  rightButton.addEventListener('click', function() {
    adjustServoValue('horizontal', 2);
  });
}

function adjustServoValue(axis, change) {
  const slider = document.querySelector(`.servo-slider.${axis}`);
  const input = slider.closest('.slider-container').querySelector('.servo-value');

  if (!slider || !input) return;

  let currentValue = parseInt(slider.value);
  let newValue = currentValue + change;

  newValue = Math.min(Math.max(newValue, -90), 90);

  slider.value = newValue;
  input.value = newValue;
}

function initHomeButton() {
  const homeButton = document.querySelector('.servo-btn.home');
  homeButton.addEventListener('click', function() {
    resetServoToCenter('vertical');
    resetServoToCenter('horizontal');
  });
}

function resetServoToCenter(axis) {
  const slider = document.querySelector(`.servo-slider.${axis}`);
  const input = slider.closest('.slider-container').querySelector('.servo-value');

  if (!slider || !input) return;

  slider.value = 0;
  input.value = 0;
}

