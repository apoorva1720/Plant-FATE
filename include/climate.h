#ifndef PLANT_FATE_ENV_CLIMATE_H_
#define PLANT_FATE_ENV_CLIMATE_H_

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


namespace env{

class Clim{
	public:
	double tc = 25;             // temperature, deg C
	double ppfd = 600;          // umol/m2/s
	double vpd  = 1000;         // Pa
	double co2  = 400;          // ppm
	double swp = -1;		// MPa

};


class Climate{
	
	private:
	double t_prev = 0; // time at current data values (years since 2000-01-01)
	double t_next = 0;	// next time in file for which data is available
	Clim clim_prev;
	Clim clim_next;
	double t_base = 2000.0;

	public:
	double t_now;
	Clim clim;

	// environmental parameters for phydro
	double elv  = 0;            // m.a.s.l.
	
	// photosynthesis params
	double kphio = 0.087;        // quantum yield efficiency
	double rdark = 0.02;
	double fapar = 1.0;         // fractioni

	std::string metFile = "";
	std::string co2File = "";
	bool interpolate = false;

	private:
	std::ifstream fin_met;
	std::ifstream fin_co2;

	public:
		
	int init(){
		fin_met.open(metFile.c_str());
		fin_co2.open(co2File.c_str());
		
		if (!fin_met){
			std::cerr << "Error: Could not open file " << metFile << "\n";
			return 1;
		}
		if (!fin_co2){
			std::cerr << "Error: Could not open file " << metFile << "\n";
			return 1;
		}
		
		// skip header
		std::string line;
		getline(fin_met, line);
		getline(fin_co2, line);

		// read first entry into t_prev
		readNextLine_met();
		t_now = t_prev = t_next;
		clim = clim_prev = clim_next;
		readNextLine_met();
		
		return 0;

	}

	
	Clim interp(Clim &clim_prev, Clim &clim_next){
		return clim_prev;
	}

	void updateClimate(double t){
		if (t == t_now) return;
		
		if (t > t_prev && t < t_next){
			clim = interp(clim_prev, clim_next);
		}

		while (t >= t_next){
			std::cout << "update - " << t << " --> " << t_next << "\n";
			clim_prev = clim_next;
			t_prev = t_next;
			readNextLine_met();
		}
	
		clim = interp(clim_prev, clim_next);		
	}
	

	template<class T> 
	T as(std::string s){
		std::stringstream sin(s);
		T data;
		sin >> data;
		return data;
	}

	int readNextLine_met(){

		//std::vector<std::string>   result;
		std::string                line;
		std::getline(fin_met, line);

		std::stringstream          lineStream(line);
		std::string                cell;
		
		if (fin_met.eof()){
			fin_met.clear();
			fin_met.seekg(0);
			std::getline(fin_met, line); // skip header
			t_base = t_next;
		}

		std::getline(lineStream, cell, ',');
		int year = as<int>(cell);
		
		std::getline(lineStream, cell, ',');
		int month = as<int>(cell);
		
		std::getline(lineStream, cell, ',');
		clim_next.tc = as<double>(cell);
		
		std::getline(lineStream, cell, ',');
		clim_next.vpd = as<double>(cell);
		
		std::getline(lineStream, cell, ',');
		clim_next.ppfd = as<double>(cell);

		std::getline(lineStream, cell, ',');
		clim_next.swp = as<double>(cell);
	
		t_next = year-t_base + (month-1)/12.0;
			
		return 0;
	}


	void print(double t){
		int year = 2000 + int(t);
		double month = (t-int(t))*12+1;
		int yearp = 2000 + int(t_prev);
		double monthp = (t_prev-int(t_prev))*12+1;
		int yearn = 2000 + int(t_next);
		double monthn = (t_next-int(t_next))*12+1;
		std::cout << "Climate at t = " << year << "." << month << "\n";
		std::cout << "prev: " << yearp << "." << monthp << " | " << clim_prev.vpd << " " << clim_prev.ppfd << "\n"; 
		std::cout << "now : " << year  << "." << month  << " | " << clim.vpd      << " " << clim.ppfd      << "\n"; 
		std::cout << "next: " << yearn << "." << monthn << " | " << clim_next.vpd << " " << clim_next.ppfd << "\n"; 
	}
};





} // namespace env


#endif