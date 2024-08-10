#include <iostream>
#include <fstream>
#include <random>
#include <cstring>
#include <getopt.h>
#include <cstdint>

// Enumeración para las opciones de tamaño
enum SizeOption {
	SMALL = 512 * 1024 * 1024,  // 512 MB
	MEDIUM = 1024 * 1024 * 1024, // 1 GB
	LARGE = 2147483648 // 2 GB in bytes (2 * 1024 * 1024 * 1024)
};

// Función para parsear el argumento de tamaño
SizeOption parseSize(const std::string& sizeStr) {
	if (sizeStr == "SMALL") {
		return SMALL;
	} else if (sizeStr == "MEDIUM") {
		return MEDIUM;
	} else if (sizeStr == "LARGE") {
		return LARGE;
	} else {
		throw std::invalid_argument("Invalid size argument.");
	}
}

// Función para generar el archivo binario
void generateFile(SizeOption size, const std::string& outputPath) {
	// Abrir archivo en modo binario
	std::ofstream outFile(outputPath, std::ios::out | std::ios::binary);
	if (!outFile) {
		throw std::runtime_error("Failed to open output file.");
	}
	
	// Generador de números aleatorios
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(0, std::numeric_limits<int>::max());
	
	// Calcular la cantidad de enteros a generar
	size_t numIntegers = size / sizeof(int);
	for (size_t i = 0; i < numIntegers; ++i) {
		int randomNumber = dist(gen);
		outFile.write(reinterpret_cast<char*>(&randomNumber), sizeof(int));
	}
	
	outFile.close();
}

int main(int argc, char* argv[]) {
	int opt;
	std::string sizeStr;
	std::string outputPath;
	
	// Definición de las opciones de línea de comando
	static struct option long_options[] = {
		{"size", required_argument, 0, 's'},
	{"output", required_argument, 0, 'o'},
		{0, 0, 0, 0}
	};
	
	// Parseo de las opciones de línea de comando
	while ((opt = getopt_long(argc, argv, "s:o:", long_options, nullptr)) != -1) {
		switch (opt) {
		case 's':
			sizeStr = optarg;
			break;
		case 'o':
			outputPath = optarg;
			break;
		default:
			std::cerr << "Usage: " << argv[0] << " --size <SIZE> --output <OUTPUT FILE PATH>\n";
			return 1;
		}
	}
	
	// Verificación de argumentos
	if (sizeStr.empty() || outputPath.empty()) {
		std::cerr << "Usage: " << argv[0] << " --size <SIZE> --output <OUTPUT FILE PATH>\n";
		return 1;
	}
	
	// Generación del archivo binario
	try {
		SizeOption size = parseSize(sizeStr);
		generateFile(size, outputPath);
		std::cout << "File generated successfully.\n";
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << '\n';
		return 1;
	}
	std::cout << "Press Enter to exit...";
	std::cin.get();
	
	return 0;
}
