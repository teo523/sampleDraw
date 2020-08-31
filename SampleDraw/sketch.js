//var socket;
var lineThickness;
var lineColor = [];
var cursorX;
var cursorY;
var headX=0;
var speed = 10;
var tracks = [];
var option = 1;
var bHeight;
var tWidth;
var tHeight;
var bWidth;
let vOffset;
var canv;
var thick = 3;
var isPlaying = 0;
var points = 0;
let counter = 0;
var id = 0;
var colors = [[255,255,0],[255,0,255],[0, 255, 255],[200,200,200]];


function setup() {


    canv = createCanvas(0.95*windowWidth, windowHeight);
    //canv.style("z-index","1");
    background(255,255,255);
    //canv.hide();
    //makeDisplay(false);
    
 
   
    
    bHeight=height/6;
    tWidth=width/10;
    tHeight=height/10;
    bWidth= width/4;
    vOffset = (height - bHeight - 8*tHeight)/2;
  
   /* slider = createSlider(1, 50, 10);
    slider.position(4*bWidth+3*vOffset ,height-0.8* bHeight / 3);
    slider.style("width", "8vw");
     slider.style("background", "red");
   */

}





/*function newDrawing(data) {
    noStroke();
    //fill(data.color[0], data.color[1], data.color[2]);
}

function highlight(){
    text2.style("background-color","rgba(255,0,0,0.3)");
}
function unhighlight(){
    text2.style("background-color","white");
}
*/


function mouseClicked() {

    
    

    //if we are in the drawing rectangle, then create an osc
    if(mouseY<=5*height/6 && mouseX<width){
    	if (points == 0) {
    		points = 1;
	        cursorX = mouseX;
	        cursorY = mouseY;
    		tracks.push(new Track(mouseX,mouseY,mouseX,mouseY,thick,option));
    	}
    	
    	else  {
    		id++;
    		tracks[tracks.length-1].update(mouseX,mouseY);
    		points = 0;
    		
    	}
        
        
         

    }
    
    
    
    
    //lower bar button update
    else {
        if (mouseX<bWidth)
            option = 0;
        else if (mouseX>bWidth && mouseX<2*bWidth)
            option = 1;
        else if (mouseX>2*bWidth && mouseX<3*bWidth)
            option = 2;
        else if (mouseX>3*bWidth && mouseX<4*bWidth)
            option = 3;
        else if (mouseY > height - bHeight + vOffset/2 && mouseY < height - bHeight + vOffset/2 + 2*bHeight/3 - vOffset && mouseX > 4 * bWidth + vOffset && mouseX < 4 * bWidth + vOffset + (width - (4 * bWidth + vOffset))/2) 
            isPlaying = 1 - isPlaying;
        else if (mouseY > height - bHeight + vOffset/2 && mouseY < height - bHeight + vOffset/2 + 2*bHeight/3 - vOffset &&  mouseX > 4 * bWidth + vOffset + (width - (4 * bWidth + vOffset))/2 && mouseX < width) 
                {
                   

                    //submitButton();

        }
      
//rect(4 * bWidth + vOffset, height - bHeight + vOffset/2,(width - (4 * bWidth + vOffset))/2,2*bHeight/3 - vOffset);

    
    }


}




function mouseDragged() {

   
   }



function draw() {
    background(0,20,0);
  
    counter = Bela.data.buffers[0];
    
   
    
    //slider.position(4*bWidth+3*vOffset ,height-0.8* bHeight / 3);
    //speed = slider.value();

    if (mouseIsPressed && mouseY<=5*height/6 && mouseX<=width-tWidth) {
        mouseDragged();

    }

    menuButtons();
    stroke(255,255,255);
    line(counter * width,0,counter * width,height-bHeight);
    
    if (headX>=width-tWidth)
        headX=0;
   

    for (var i = 0; i < tracks.length; i++) {
   
        tracks[i].show();

  }
 
}



// Daniel Shiffman
// code for https://youtu.be/vqE8DMfOajk

function Track(x0, y0, x1, y1, thick, option) {
  
    this.x0 = x0;
    this.y0 = y0;
    this.x1 = x1;
    this.y1 = y1;
    this.thick = thick;
    this.type = option;

    this.update = function(x,y) {
    this.x1 = x;
    this.y1 = y;
    var buffer = [id,this.x0/width,this.y0/height,this.x1/width,this.y1/height, this.type];
	Bela.data.sendBuffer(0, 'float', buffer);
	
  };

  this.show = function() {
    stroke(colors[this.type]);
   
      fill(colors[this.type]);
      ellipse(this.x0, this.y0, 5, 5);
      ellipse(this.x1, this.y1, 5, 5);
      line(this.x0,this.y0,this.x1,this.y1);
    
    }
  };





function menuButtons() {

    bHeight=height/6;
    tWidth=width/10;
    tHeight=height/10;
    bWidth= width/4;
    noStroke();
    strokeWeight(4);
    
    //paint bottom bar rectangles with white frame if they are selected
    fill(colors[0]);
    if (option == 0)
    stroke(255);
    
    rect(0,height - bHeight,bWidth,bHeight);
    
    noStroke();
	
    if (option == 1)
        stroke(255);
    fill(colors[1]);
    rect(bWidth,height - bHeight,bWidth,bHeight);
    
    noStroke();

    if (option == 2) 
        stroke(255);
    fill(colors[2]);
    rect(2*bWidth,height - bHeight,bWidth,bHeight);
    noStroke();

    if (option == 3 )
        stroke(255);
    fill(colors[3]);
    rect(3*bWidth,height - bHeight,bWidth,bHeight);
    noStroke();

   
    //draw slider background and play button
    /*
    fill(190);
    rect(4 * bWidth, height - bHeight,width - 4 * bWidth ,bHeight);

    fill(0);
    rect(4 * bWidth + vOffset, height - bHeight/3,width - (4 * bWidth + vOffset),bHeight/3);
    if (isPlaying)
        fill(255,0,0);
    else 
        fill(0,255,0);

    rect(4 * bWidth + vOffset, height - bHeight + vOffset/2,(width - (4 * bWidth + vOffset))/2,2*bHeight/3 - vOffset);

  */

    
   
}

