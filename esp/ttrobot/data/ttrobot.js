var rangeSlider = function () {
  var rangeEls = document.getElementsByClassName('range-slider__range');
  
  Array.from(rangeEls).forEach((range) => {
    //set the value elements to the initial vals
    range.nextElementSibling.innerHTML = range.value;

    //add a oninput event handler to update the html of the value Element with the value of the range        
    range.addEventListener("input", function () {
      this.nextElementSibling.innerHTML = this.value;      
    });
    range.addEventListener("change", function () {      
      setmotor(this.id, this.value);
    });
  });
};

rangeSlider();

function setmotor(motorName, speed) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {      
      console.log("mcu set value to: " + this.responseText);
    }
  };
  xhttp.open("GET", "/setmotor?m="+motorName + "&s="+ speed, true);
  xhttp.send();
}