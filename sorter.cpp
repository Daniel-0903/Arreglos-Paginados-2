#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <ctime>
#include <chrono>

// Clase PagedArray
class PagedArray {
private:
	std::ifstream inputFile;
	std::ofstream outputFile;
	int pageSize; // Tamaño de la página en enteros
	int numPages; // Número total de páginas
	std::unordered_map<int, std::vector<int>> pageCache; // Caché para las páginas cargadas
	std::vector<int> pageUsage; // Para llevar el seguimiento de la antigüedad de las páginas
	
	void loadPage(int pageIndex) {
		// Si la página ya está cargada, no hacemos nada
		if (pageCache.find(pageIndex) != pageCache.end()) {
			return;
		}
		
		// Si el caché está lleno, descargamos una página
		if (pageCache.size() >= 4) {
			int oldestPageIndex = pageUsage.front();
			writePageToFile(oldestPageIndex);
			pageCache.erase(oldestPageIndex);
			pageUsage.erase(pageUsage.begin());
		}
		
		// Cargar la página desde el archivo
		std::vector<int> page(pageSize);
		inputFile.seekg(pageIndex * pageSize * sizeof(int), std::ios::beg);
		inputFile.read(reinterpret_cast<char*>(page.data()), pageSize * sizeof(int));
		
		if (!inputFile) {
			throw std::runtime_error("Error reading page from file.");
		}
		
		pageCache[pageIndex] = page;
		pageUsage.push_back(pageIndex);
	}
	
	void writePageToFile(int pageIndex) {
		if (pageCache.find(pageIndex) != pageCache.end()) {
			outputFile.seekp(pageIndex * pageSize * sizeof(int), std::ios::beg);
			outputFile.write(reinterpret_cast<const char*>(pageCache[pageIndex].data()), pageSize * sizeof(int));
			
			if (!outputFile) {
				throw std::runtime_error("Error writing page to file.");
			}
		}
	}
	
public:
		PagedArray(const std::string& inputPath, const std::string& outputPath, int pageSize)
			: pageSize(pageSize) {
			inputFile.open(inputPath, std::ios::in | std::ios::binary);
			outputFile.open(outputPath, std::ios::out | std::ios::binary | std::ios::trunc);
			
			if (!inputFile.is_open() || !outputFile.is_open()) {
				throw std::runtime_error("Unable to open input or output file.");
			}
			
			// Calcular el número total de páginas
			inputFile.seekg(0, std::ios::end);
			std::streampos fileSize = inputFile.tellg();
			numPages = (fileSize + pageSize * sizeof(int) - 1) / (pageSize * sizeof(int));
			inputFile.seekg(0, std::ios::beg);
		}
		
		~PagedArray() {
			// Escribir todas las páginas restantes al archivo de salida
			for (const auto& entry : pageCache) {
				writePageToFile(entry.first);
			}
			inputFile.close();
			outputFile.close();
		}
		
		int& operator[](int index) {
			int pageIndex = index / pageSize;
			int offset = index % pageSize;
			
			loadPage(pageIndex);
			auto it = std::find(pageUsage.begin(), pageUsage.end(), pageIndex);
			if (it != pageUsage.end()) {
				pageUsage.erase(it);
			}
			pageUsage.push_back(pageIndex);
			
			return pageCache[pageIndex][offset];
		}
		
		int size() const {
			return numPages * pageSize;
		}
};

void quickSort(PagedArray& arr, int low, int high) {
	if (low < high) {
		int pivot = arr[(low + high) / 2];
		int i = low;
		int j = high;
		
		while (i <= j) {
			while (arr[i] < pivot) i++;
			while (arr[j] > pivot) j--;
			if (i <= j) {
				std::swap(arr[i], arr[j]);
				i++;
				j--;
			}
		}
		
		if (low < j) quickSort(arr, low, j);
		if (i < high) quickSort(arr, i, high);
	}
}
void insertionSort(PagedArray& arr, int n) {
	for (int i = 1; i < n; i++) {
		int key = arr[i];
		int j = i - 1;
		while (j >= 0 && arr[j] > key) {
			arr[j + 1] = arr[j];
			j--;
		}
		arr[j + 1] = key;
	}
}
void bubbleSort(PagedArray& arr, int n) {
	for (int i = 0; i < n - 1; i++) {
		for (int j = 0; j < n - i - 1; j++) {
			if (arr[j] > arr[j + 1]) {
				std::swap(arr[j], arr[j + 1]);
			}
		}
	}
}

int main(int argc, char* argv[]) {
	if (argc < 7) {
		std::cerr << "Usage: " << argv[0] << " --input <INPUT FILE PATH> --output <OUTPUT FILE PATH> --alg <ALGORITMO>\n";
		return 1;
	}
	
	std::string inputPath, outputPath, algorithm;
	int pageSize = 1024;  // Por ejemplo, 1024 enteros por página
	
	for (int i = 1; i < argc; i += 2) {
		std::string arg = argv[i];
		if (arg == "--input") {
			inputPath = argv[i + 1];
		} else if (arg == "--output") {
			outputPath = argv[i + 1];
		} else if (arg == "--alg") {
			algorithm = argv[i + 1];
		} else {
			std::cerr << "Unknown argument: " << arg << "\n";
			return 1;
		}
	}
	
	try {
		PagedArray arr(inputPath, outputPath, pageSize);
		
		// Medición del tiempo de ordenamiento
		auto start = std::chrono::high_resolution_clock::now();
		
		if (algorithm == "QS") {
			quickSort(arr, 0, arr.size() - 1);
		} else if (algorithm == "IS") {
			insertionSort(arr, arr.size());
		} else if (algorithm == "BS") {
			bubbleSort(arr, arr.size());
		} else {
			std::cerr << "Unknown algorithm: " << algorithm << "\n";
			return 1;
		}
		
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> duration = end - start;
		
		std::cout << "Sorting completed in " << duration.count() << " seconds.\n";
		std::cout << "Algorithm used: " << algorithm << "\n";
		// Aquí deberías imprimir la cantidad de page faults y page hits
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << '\n';
		return 1;
	}
	
	return 0;
}


//prueba
