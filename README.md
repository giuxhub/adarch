# ADARCH
*Development of active defense tools, made easier.*


`ADARCH` (from Active Defense ARCHitecture) is a cross-platform prototype of a framework that makes active defense tools development and deployment easier. 
The goal of ADARCH is to simplify the implementation of common functionalities of active defence tools, as well as to provide a uniform interface for the user to interact with the tools. The framework uses a modular approach to add and remove active defence tools, and aims to provide seamless integration.

The project and its architecture are described by our paper [Towards Intelligent Cyber Deception Systems](https://doi.org/10.1007/978-3-030-02110-8_2).


**It simplify the creation of active defense tools because**: 

* it implements functions that are common to multiple active defence tools;
* it provides a Python API to access common functions;
* it implements the Python Interpreter to let the user develop active defense tools using Python language;
* it can be configured using a simple JSON file;

### Installing

    An easy-to-use CMakeLists.txt is provided.
	
### Usage

The active defense tools are configured using an `adarch.conf` file with JSON format. Example:

	{
		"tool_1": {
			"file": "Portspoof",
			"class": "Portspoof",
			"method": "start",
			"ports": [4444]
		}

		"tool_2": {
			"file": "Tcprooter",
			"class": "Tcprooter",
			"method": "start",
			"ports": [5000]
		}
	}
	
Inside the `test` directory, for each supported OS distribution, there is a `Dockerfile` to test `ADARCH` (for the moment there is only the Linux version).

**In order to test ADARCH using a Docker container**: 

* **put** inside `test/adarch` the compiled binary, the tools and the configuration file
* **build** Docker Image `docker build -t adarch .`
* **parse** AppArmor rules `apparmor_parser -r -W docker-network`
* **run** the container `docker run --rm -it -p 4444:4444 -p 5000:5000 --security-opt apparmor=docker-network adarch`

### License

`ADARCH` was made with ♥ and it is released under the GPL 3 license.
