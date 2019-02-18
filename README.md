# Power-traces-analysis-on-CC2642R1
Power traces prediction on CC2642R1

## The larger problem or challenge being addressed
The acquisition and analysis of power traces is very important. By analyzing the power trace, we can know the power resources consumed by an electric appliance during a certain period of time. At present, to predict the electrical appliances to which the power trace belongs, it is often required to send the original power trace data to the cloud, which unnecessarily consumes network resources. In order to solve this problem, this project proposes the possibility of power trajectory prediction on embedded device itself.

## A description of the project
The project contains two parts: training the power trace prediction model and making predictions. The training part is achieved using python and ran on laptop. Since it is not the focus of this project, we are not going to describe the details. 

The prediction part is realized on the TI LaunchPad CC26X2R1. Basically, this part consists of data preprocessing, prediction on neural network and data transmissions. The data model of the prediction on CC26X2R1 is shown in the figure below. The data flow can be described as follows:

![Model](https://github.com/Enihsuns/Power-traces-analysis-on-CC2642R1/blob/master/img/model.png)

1. Input. The input only contains a float. It represents in the power value in a second. Since I’m not using the embedded device for real power sensing, here I simply use the Universal Asynchronous Receiver/Transmitter driver to simulate the process of reading data from a power sensor.
2. Save the input into a value buffer. The value buffer saves power values from the last 60 seconds. Therefore, its size is as large as 60 floats. The buffer is following the rule similar to a queue. When a new value comes in, the oldest value will be pop out.
3. Make a prediction. The prediction model is a Neural Network, with two layers of size 128 and 9. The input is the whole value buffer. After going through the neural network, an int value which represents the prediction of electric appliance will be generated.
4. Output. The output contains a float and an int. The float is the oldest value in the value buffer, while the int is the prediction value.

## The technical challenges that make this challenging
1.	The limited size of memory. The device should save the Neural Network model. But the device only contains 80KB of SRAM. 
2.	Time requirement. The program is expected to give an answer in every second. Therefore, all the operations need to finish in a second.

##	What hardware and software platforms you used?
Hardware: TI LaunchPad kit with SimpleLink Wireless MCU LAUNCHXL-CC26X2R1

Software: Code Composer Studio (CCS) from Texas Instruments.

##	How the software platform / OS helped or didn’t help with creating the project
**Helpful:**

CCS provides many useful features for debugging the program and monitoring the status. For example, when loading the Neural Network model through a file I/O operation, I use the memory browser to check whether the weights and bias matrices are successfully assigned.
Another useful feature is that CCS provides a file named CC26X2R1_LAUNCHXL.cmd, which enables users to set stack size, heap size, and some other parameters. I use it to set the heap size of 62KB so that it is enough to save my Neural Network model.

**Not very helpful:**

In this project, I used the Universal Asynchronous Receiver/Transmitter driver provided in the CC26X2R1 SDK to simulate the process of receiving data from sensors and sending classification results. At the same time, I use some standard I/O operations in C (like printf and puts) for debugging. However, in the setting of CCS debugger, the standard I/O operations use the same UART driver to send data. Therefore, I cannot use standard I/O functions and the UART driver at the same time. This makes the debugging process very inconvenient.
