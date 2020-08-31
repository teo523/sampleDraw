/** ECS7012
 * Music and Audio Progrmming
 * Final Project 
 * Teodoro Dannemann
 * 
 * Here we present the code of SampleDraw, a tool that allows the visual manipulation 
 * of samples in the time-pitch domain. Firstly, users upload their custom samples to Bela. Then, through Bela GUI, 
 * users can draw lines in a time-pitch space, where each line corresponds to a specific sample playback. 
 * Samples are manipulated in the time domain for changing their duration and pitch according to the drawn lines. 
 * We used a simple method based on circular buffers that allows both time-scale modification and also pitch-scale 
 * modification of the samples. The tool allows the simultaneous playing of several samples at the same time, and allows 
 * the user to upload up to four different samples.
**/

#include <Bela.h>
#include <SampleLoader.h>
#include <math.h>   
#include <libraries/Gui/Gui.h>
#include <Gpio.h>


//Create a Gui object
Gui gui;

//we will use this GPIO output to analyze the processing speed of render function
Gpio gpioOut;
int gOutputPin = 89;

//A vector of vectors. Each time a user draws a stroke in the GUI, we receive the data as a vector that we append to this object
std::vector<std::vector<float>> strokes;

//The number of sound samples uploaded by the user. We specify their names in gFilename array
const int N = 4;
string gFilename [N] = {"Cello.wav","waves.wav","trumpet.wav","Amazed.wav"}; // Name of the sound file (in project folder)
float *gSampleBuffer [N];			 // Buffer that holds each sound file
int gSampleBufferLength [N];		 // The length of each buffer in frames

//We will use up to 10 circular buffers (here we call them delay buffers). Each buffer will have attached to it:
// (1) a fractional read pointer (gReadPointer) and its integer part (gReadPointerInt) which points at the last frame of the sound sample we are currently reading
// (2) a write pointer (gDelayBufferWritePointer) that points to the frame within the circular buffer we are writin in 
// (3) a fractional read pointer (gDelayBufferReadPointer), as within the circular buffer we can advance in non-integer steps
// (4) an integer read pointer (gDelayBufferReadPointerInt1) that looks for the closest frame (integer) to the above fractional read pointer
// (5) a second integer read pointer (gDelayBufferReadPointerInt2) that keeps track of the frame that is 180º apart from the above read pointer

const int gDelayBufferLength [30] = {3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000};
float gDelayBuffer [30][3000];
float gReadPointer [30] ={0}; // (1)
int gReadPointerInt [30] = {0};	// (1)
int gDelayBufferWritePointer[30] = {0}; // (2)
float gDelayBufferReadPointer[30] = {0}; // (3)
int gDelayBufferReadPointerInt1[30] = {0}; // (4)
int gDelayBufferReadPointerInt2[30] = {0}; // (5)


// as well, we use an auxiliary array to attach active stroke with any available circular buffer
int gDelayBufferForStroke[30] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

// we define an overlap area. if the number of frames between the write pointer and any of the two read pointers of the circular 
// buffer is less than this overlap, ten we apply the crossfade
int overlap = 100;

//crossfade factor for each circular buffer
float crossfade[30] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//How much the read pointers of the circular buffer will move forward (if pitch remains unaltered this equals to one)
float pitchHop[30]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

//we assign an id for each received stroke. This variable will increase each time we receive a new stroke
int id = 0;

//we must scale the samples to the corresponding duration 
float scaleFactor = 1.0;

// global time 
int gMetro = 0; 
int gDuration = 10; //duration in seconds;

bool setup(BelaContext *context, void *userData)
{

	gpioOut.open(gOutputPin, OUTPUT); // Open the pin as an output
	
	// Set up the GUI
	gui.setup(context->projectName);
	gui.setBuffer('f', 6); // we will receive a buffer of 6 floats
    
	// this part, for loading the sound files, is taken from class examples
	// Check the length of the audio file and allocate memory
	for (int i=0;i<N;i++) {
    	gSampleBufferLength[i] = getNumFrames(gFilename[i]);
    
    	if(gSampleBufferLength[i] <= 0) {
    		rt_printf("Error loading audio file '%s'\n", gFilename[i].c_str());
    		return false;
    	}
    
    	gSampleBuffer[i] = new float[gSampleBufferLength[i]];
    
    	// Make sure the memory allocated properly
    	if(gSampleBuffer[i] == 0) {
    		rt_printf("Error allocating memory for the audio buffer.\n");
    		return false;
		}
    
    	// Load the sound into the file (note: this example assumes a mono audio file)
		getSamples(gFilename[i], gSampleBuffer[i], 0, 0, gSampleBufferLength[i]);
	
    	rt_printf("Loaded the audio file '%s' with %d frames (%.1f seconds)\n", 
    			gFilename[i].c_str(), gSampleBufferLength[i],
    			gSampleBufferLength[i] / context->audioSampleRate);
		}
	return true;
}

void render(BelaContext *context, void *userData)
{

	//we read data buffers sent from the GUI
	//We store the DataBuffer in 'buf'
	DataBuffer& buf = gui.getDataBuffer(0);
	
	// Retrieve contents of the buffer as floats
	float * data = buf.getAsFloat();
	
	//Only read if the id has not been added yet
    if (data[0]>(float)id){
    	
		id++;
		//scale the duration of the sample by the length of the drawn stroke
		float playDuration = (data[3]-data[1])*gDuration;
		scaleFactor = playDuration/((float)gSampleBufferLength[(int)data[5]]/(float)context->audioSampleRate);
		
		//we add the received data as a vector. We always store the minimum x coordinate in the first location of row
		vector<float> row = {};
		if (data[1]<data[3])
			row = {data[1],data[2],data[3],data[4],data[5],scaleFactor,0.0};
        else
        	row = {data[3],data[4],data[1],data[2],data[5],-scaleFactor,0.0};
        	
        //add to strokes
		strokes.push_back(row);
	  

    }
    
    //We also send to the GUI the current value of time (every 100 time steps)
	 if (gMetro % 100 == 0)
		gui.sendBuffer(0, (float)gMetro/(float)((context->audioSampleRate) * gDuration));


   
	
    
    for(unsigned int n = 0; n < context->audioFrames; n++) {
		
		//This line is uncommented for the evaluation part. It writes to the GPIO in order to measure in a scope the processing time of the code
		//gpioOut.write(1);
    	
    	
    	gMetro++; //increase our time counter and reset if we got to the end 
    	if (gMetro >= gDuration * context->audioSampleRate)
    		gMetro = 0;
    
    	float out = 0;
      
       //deactivate strokes and reset pointers
        for (int j = 0; j< 10; j++){
        	if (gDelayBufferForStroke[j]!=-1){
        		if (strokes[gDelayBufferForStroke[j]][0] > (float)gMetro/(float)((context->audioSampleRate) * gDuration)  || strokes[gDelayBufferForStroke[j]][2]<(float)gMetro/((float)(context->audioSampleRate) * gDuration)) {
        			strokes[gDelayBufferForStroke[j]][6]=0; //stroke inactive
        			gDelayBufferForStroke[j]=-1; //free the circular buffer attached to it
        			//Reset read and write pointers
        			gReadPointer[j]=0; 
        			gReadPointerInt[j]=0;
        			gDelayBufferWritePointer[j] = 0;
        			gDelayBufferReadPointer[j] = 0;
        			
        		}  		
        	}    
        }
        
     
        //activate strokes and look for a circular buffer to attach to it. 
        for(std::size_t i = 0; i < strokes.size(); i++) {
        	if ((float)gMetro/(float)((context->audioSampleRate) * gDuration) >= strokes[i][0]  && strokes[i][2]>=(float)gMetro/((float)(context->audioSampleRate) * gDuration) && strokes[i][6]==0) {
        		//assign a free circular buffer 
        		for (int j = 0; j< 10; j++){
        			if (gDelayBufferForStroke[j]==-1){
        				gDelayBufferForStroke[j]=i;
        				strokes[i][6]=1;
        				break;
        			}
        		}
        	}
        }
        
    
         for(std::size_t i = 0; i < 10; i++) {
        	//gDelayBufferForStroke[i] gives us the row of strokes that buffer i is pointing to
        	int j = gDelayBufferForStroke[i];
         
        	//if the stroke is active 
	         if (j>=0){
			 	
	        	
	        //the specific sample for that stroke is given in the index 4 of the vector		
	    	int sample = (int) strokes[j][4];
	    	
	    	//We calculate the current (x,y) position of the stroke that is being touched by the play head. 
	    	//we simply inerpolate between the extremes of the strokes (x0,y0) and (x1,y1)
	    	float y0 = strokes[j][1]; 
	    	float y1 = strokes[j][3];
	    	float x0 = strokes[j][0];
	    	float x1 = strokes[j][2];
	    	
	    	float x = (float)gMetro/(float)((context->audioSampleRate) * gDuration);
	    	float y = 1- ((x - x0) * (y1-y0) / (x1-x0) + y0);
	    	
	    	//We map the y coordinate to the specific hop the read pointers will make, i.e., how many frames will they jump
	    	pitchHop[i]= strokes[j][5]*(0.01+0.99*y/0.5);
	
	    	//We save the current frame to be read from the sample sound buffer	
	        float in = gSampleBuffer[sample][gReadPointerInt[i]];
	           
			//we advance the read pointer of the sound sample, and take the integer part. How much to advance the pointer is given by the scale factor
	        gReadPointer[i] += 1/strokes[j][5];
	        gReadPointerInt[i] = (int)round(gReadPointer[i]);
	        if (gReadPointerInt[i] >= gSampleBufferLength[sample])
	        	gReadPointer[i] = 0;
	        	
	        // The write pointer points to the oldest sample in the buffer
	        // We overwrite the oldest sample with the newest one
	        gDelayBuffer[i][gDelayBufferWritePointer[i]] = in;
	        
	
	        //We force the second read pointer of the circular buffer to be in a 180º phase with respect of the first read pointer. 
	        //This means that the distance between these two pointers will always be gDelayBufferLength[i]/2
	        gDelayBufferReadPointerInt2[i] = (((int)gDelayBufferReadPointerInt1[i] + gDelayBufferLength[i] - gDelayBufferLength[i]/2) % gDelayBufferLength[i]);
	        
	        //Check if first read pointer is in the overlap zone (approaching writepointer)
	        if (((gDelayBufferWritePointer[i] - gDelayBufferReadPointerInt1[i] <= overlap && gDelayBufferWritePointer[i] - gDelayBufferReadPointerInt1[i] >= 0) ||
	            (gDelayBufferWritePointer[i] - gDelayBufferReadPointerInt1[i] <= 0 && gDelayBufferWritePointer[i] <= overlap && gDelayBufferReadPointerInt1[i] >= gDelayBufferLength[i] - overlap))
	            && pitchHop[i] != 1) {
	            
	            
	            //if so, apply crosssfade	
	            int diff = gDelayBufferWritePointer[i] - gDelayBufferReadPointerInt1[i];
	            if (diff < 0)
	            	diff = gDelayBufferLength[i] - gDelayBufferReadPointerInt1[i] + gDelayBufferWritePointer[i];
	            if (diff > overlap)
	            	diff = overlap;
	            
	            //We calculate the crossfade factor. Note that when gDelayBufferWritePointer[i] = gDelayBufferReadPointerInt1[i], then the factor is 0,
	            //so the first read pointer will be neglected and only the second one will be taken into account
	            crossfade[i] = (float)diff/(float)overlap;
	            
	        }
	        
	        //Now we do the same for the second read pointer
	        if (((gDelayBufferWritePointer[i] - gDelayBufferReadPointerInt2[i] <= overlap && gDelayBufferWritePointer[i] - gDelayBufferReadPointerInt2[i] >= 0) ||
	            (gDelayBufferWritePointer[i] - gDelayBufferReadPointerInt2[i] <= 0 && gDelayBufferWritePointer[i] <= overlap && gDelayBufferReadPointerInt2[i] >= gDelayBufferLength[i] - overlap))
	            && pitchHop[i] != 1) {
	            
	            //apply crosssfade	
	            int diff = gDelayBufferWritePointer[i] - gDelayBufferReadPointerInt2[i];
	            if (diff < 0)
	            	diff = gDelayBufferLength[i] - gDelayBufferReadPointerInt2[i] + gDelayBufferWritePointer[i];
	            if (diff > overlap)
	            	diff = overlap;
	            
	            crossfade[i] = 1 - (float)diff/(float)overlap;
	        
	        }
	        
	        //We write to the output the weighted sum of the frames pointed by both read pointers
	        out += crossfade[i] * gDelayBuffer[i][gDelayBufferReadPointerInt1[i]] + (1 - crossfade[i]) * gDelayBuffer[i][gDelayBufferReadPointerInt2[i]];
	       
			// Advance and wrap the pointers
	        gDelayBufferWritePointer[i]++;
	        if(gDelayBufferWritePointer[i] >= gDelayBufferLength[i])
	        	gDelayBufferWritePointer[i] = 0;
	       	
	         //the circular boffer read pointers step is given by pitchHop which, in turn, is given by the instantaneous y. coordinate of the stroke being read.
	         gDelayBufferReadPointer[i] += pitchHop[i];
	         gDelayBufferReadPointerInt1[i] = (int)round(gDelayBufferReadPointer[i]);
	        if(gDelayBufferReadPointerInt1[i] >= gDelayBufferLength[i])
	        	gDelayBufferReadPointer[i] = 0;
			
        	}
        }
        
        // Write the output and input to different channels
		audioWrite(context, n, 0, out/(float)2);
		audioWrite(context, n, 1, out/(float)2);
		
		//This line is uncommented for the evaluation part. It writes to the GPIO in order to measure in a scope the processing time of the code
		//gpioOut.write(0);
    }
    
   
 
}

void cleanup(BelaContext *context, void *userData)
{
	// Free the buffer we created to load the WAV file
    for (int i=0;i<2;i++)
    delete[] gSampleBuffer[i];
}
