/** ECS7012
 * Music and Audio Progrmming
 * Final Project 
 * Teodoro Dannemann
 * 
 * This code is for evaluation of algorithm performance. It is meant to use in a second Bela that will work as a Scope
 * You need to wire the first Bela (running the code, uncommenting evluation lines) with the second one that will run this code.
 * 
 * - Connect GPIO 89 of the first Bela with GPIO 87 of the second one (check https://github.com/BelaPlatform/Bela/wiki/Pins-used-by-Bela)
 * - Run your code in both Bela's 
 * - This code will print the average time spend in calculations by the first Bela, out of 100000 runs
 * - You can also open the scope to visualize the actibity of the first Bela
**/


#include <Bela.h>
#include <libraries/Scope/Scope.h>
#include <Gpio.h>

// instantiate the scope
Scope scope;


Gpio gpioIn;
int gInputPin = 87; // this is the GPIO number. Check https://github.com/BelaPlatform/Bela/wiki/Pins-used-by-Bela what pin it is on your board
int gCount = 0; //counts elapsed samples
bool gStatus = 0;
bool readValue = 0;
int zeros = 0;
int ones = 0;

bool setup(BelaContext *context, void *userData)
{	
	scope.setup(1, context->audioSampleRate);
	gpioIn.open(gInputPin, INPUT); // Open the pin as an output
	return true;
}

void render(BelaContext *context, void *userData)
{
	
	for(unsigned int n = 0; n < context->audioFrames; ++n){
	
	if (gCount >= 100000){
		rt_printf("percentage: %f \n",(float)ones / ((float)zeros + (float)ones));
		zeros =0;
		ones = 0;
		gCount = 0;
		
	}
	gCount++;
	
	
	
	
	
	readValue = gpioIn.read();
	if (readValue == 0)
		zeros++;
	else if (readValue == 1)
		ones++;
	scope.log(readValue);
	}

}

void cleanup(BelaContext *context, void *userData)
{
	// Nothing to do here
}

