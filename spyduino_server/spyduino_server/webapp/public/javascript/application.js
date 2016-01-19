function makeSVG(tag, attrs) {
    var el= document.createElementNS('http://www.w3.org/2000/svg', tag);
    for (var k in attrs)
        el.setAttribute(k, attrs[k]);
    return el;
}



function show_position(x,y) {
  var transformed_x = x * window.scaleFactor;
  var transformed_y = y * window.scaleFactor;

  var circle = makeSVG('circle',{
    id: 'position',
    cx: transformed_x,
    cy: transformed_y,
    r: 200,
    stroke: "black",
    'stroke-width': 10,
    fill: "red"
  });

  document.querySelector('svg g').appendChild(circle);
}

function show_ap(x,y) {
  var transformed_x = x * window.scaleFactor;
  var transformed_y = y * window.scaleFactor;

  var circle = makeSVG('circle',{
    id: 'position',
    cx: transformed_x,
    cy: transformed_y,
    r: 200,
    stroke: "black",
    'stroke-width': 10,
    fill: "green"
  });

  document.querySelector('svg g').appendChild(circle);
}


$(document).ready(function(){
  var svg = document.querySelector('svg');
  svg.addEventListener('click',function(evt){
    function cursorPoint(evt){
      pt.x = evt.clientX; pt.y = evt.clientY;
      return pt.matrixTransform(svg.getScreenCTM().inverse());
    }

    var pt = svg.createSVGPoint();
    var loc = cursorPoint(evt);
    console.log(loc);
    var x = loc.x / window.scaleFactor
    var y = loc.y / window.scaleFactor
    alert("x: "+x.toFixed(2)+"   |    y: "+y.toFixed(2));
  },false);
})
