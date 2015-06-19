/****************************************************************************************
 * onceERRW.cpp                                               Version 1.0, Vindar, 2014 *
 *                                                                                      *
 * Implementation of onceERRW.h                                                         *
 ****************************************************************************************/


#include "../../headers/models/OnceERRW.h"
#include "../../headers/fileio.h"

namespace mylib
{
namespace models
{



	void simulationOERRW::printSimulation(std::string path,std::string filename)
		{
		out << "\n\n\n***** PRINTING SIMULATION *****\n\n";
		out << "Looking for simulation : " << path + filename << "\n\n";
		std::string f = filename; std::vector<std::string> listf;
		bool r =  mylib::ListFiles(path,f + ".L2.Z2",listf,false,true,false);
		if ((r)&&(listf.size()>0))
			{ // detailled simulation
			out << "*** DETAILLED SIMULATION FOUND !\n\n";
			extendedOERRW ew(1);
			if (ew.load(path + filename) == false) {out << "ERROR LOADING extendedOERRW FILES [" << path + filename << "]...\n\n"; input.getkey(); clearVecLat(); return;}
			out  << ew.info(); ew.plotWalk();
			if (ew.getRangeTab()->NbEntries() != 0) {ew.PlotRangeIncrease();}
			if (ew.getReturnTab()->NbEntries() != 0) {ew.PlotReturn();}
			clearVecLat();	return;
			}
		listf.clear(); clearVecLat();
		int nb = 0; r =  ListFiles(path,f + "-R*.L1.Z2",listf,false,true,false);
		if (r)  {for(int i=0;i<(int)listf.size();i++) {
					vecLat.push_back(new GrowingLatticeZ2<char,simpleOERRW::RR>(0,1)); 
					if (vecLat[i]->load(listf[i]) == false) {out << "ERROR LOADING FILE [" << listf[i] << "]..\n\n"; clearVecLat(); input.getkey(); return;}
					out << listf[i] << "\n";
					}
				}
		out << vecLat.size() << " long simulation save files found!\n\n";
		if (vecLat.size() < 2) 
			{
			out << "*** SIMPLE SIMULATION !\n\n";
			simpleOERRW sw(1.0);	if (sw.load(path + filename) == false) {out << "ERROR LOADING simpleOERRW FILE [" << path + filename << "]..\n\n"; clearVecLat(); input.getkey(); return;}
			out << sw.info() << "\n"; sw.plotWalk();
			if (sw.getRangeTab()->NbEntries() != 0) {sw.PlotRangeIncrease();}
			if (sw.getReturnTab()->NbEntries() != 0) {sw.PlotReturn();}
			clearVecLat();	return;
			}
		out << "*** LONG SIMULATION !\n\n";
		simpleOERRW w(1.0);	
		if (w.load(path + filename) == false) {out << "ERROR LOADING simpleOERRW FILE (for long sim) [" << path + filename << "]..\n\n"; clearVecLat(); input.getkey(); return;}
		out << w.info() << "\n";
		fRect ran; w.range(ran);
		LatticePlotter<const simulationOERRW,false> Plotter(*this); 
		Plotter.setRange(ran,true);
		Plotter.startPlot();
		w.plotWalk();
		if (w.getRangeTab()->NbEntries() != 0) {w.PlotRangeIncrease();}
		if (w.getReturnTab()->NbEntries() != 0) {w.PlotReturn();}
		clearVecLat();	
		return;
		}




	void simulationOERRW::longSimulation(double delta,int64 step,int64 end)
		{
		out << "\n\n\n***** LONG SIMULATION *****\n\n";
		simpleOERRW w(delta);
		std::string f = std::string("long_OERRW-d") + doubletostring_nice(delta);
		if (w.load(f)){out << "Simulation loaded !\n\n";} else {out << "Starting a new simulation\n\n";}  //try to load an exisiting simulation or start from scratch
		out << "Saving the simulation every " << step << " new sites visited\n\n";
		int64 inc = step/50; 
		while(1)
			{
			if ((end > 0)&&(w.nb_visited() >= end)) {out << "\n***** LONG SIMULATION COMPLETED *****\n\n\n"; return;}
			int64 todo = step;
			out << w.info() << "Simulating";
			while(todo >= inc) {w.makeWalk(inc); out << "."; todo -= inc;} if (todo != 0) {w.makeWalk(todo); out << ".";}
			std::string nf = f + "-R" +tostring(w.nb_visited()); 
			out << "\nsaving [" << nf << "]... ";
			w.save(f); CopyFile((f + ".L1.Z2").c_str(),(nf + ".L1.Z2").c_str(),false);
			out << " ok!\n\n" ;
			}
		}



	void simulationOERRW::detailledSimulation(double delta,int64 step,int64 end)
		{
		out << "\n\n\n***** EXTENDED SIMULATION *****\n\n";
		std::string f = "extended_OERRW-d" + doubletostring_nice(delta);
		extendedOERRW ew(delta);
		if (ew.load(f)) {out << "Simulation found. Continuing it...\n\n";} else {ew.reset(delta); out << "Starting a new simulation.\n\n";}
		int64 inc = step/50; 
		while(ew.nb_visited() < end)
			{
			int64 todo = step;
			out << ew.info()<< "\nSimulating";
			while(todo >= inc) {ew.makeWalk(inc); out << "."; todo -= inc;} if (todo != 0) {ew.makeWalk(todo); out << ".";}
			out << "\nSaving..."; ew.save(f); out << "ok!\n\n\n";
			}
		out << "\n***** EXTENDED SIMULATION COMPLETED *****\n\n\n"; return;
		return;
		}



	void simulationOERRW::packSimulation(double delta,int64 end)
		{
		out << "\n\n\n***** PACK SIMULATION *****\n\n";
		std::string filename = "pack_OERRW-d" + doubletostring_nice(delta) + "-R" + tostring(end) + ".no";
		int64 N = 1;
		std::vector<std::string> listf;	bool r =  ListFiles(".\\",filename + "*.trace.extab",listf,false,true,false);
		if ((r)&&(listf.size()!=0)) {N+= (int64)listf.size(); out << "- " << listf.size() << " previous simulation found. Continuing from there.\n\n";}
		else {out << "New pack of simulation, starting from scratch.\n\n";}	
		while(1)
			{
			out << "Simulation No " << N << "\nin progress"; 
			simpleOERRW w(delta,100000,100000);
			int64 inc = end/50; int64 todo = end;
			while(todo >= inc) {w.makeWalk(inc); out << "."; todo -= inc;} if (todo != 0) {w.makeWalk(todo); out << ".";}
			w.getRangeTab()->Save(filename + tostring(N) + ".trace.extab");
			w.getReturnTab()->Save(filename + tostring(N) + ".return.extab");
			out << "ok!\n\n" << w.info() << "\n\n";
			N++;
			}
		}


	void simulationOERRW::fusionPack(double d,int64 end)
		{
		out << "\n\n\n***** PACK FUSION *****\n\n";
		std::string outf1,outf2;
		std::string filename = "pack_OERRW-d" + doubletostring_nice(d) + "-R" + tostring(end);
			{
			std::vector<std::string> listf;
			bool r =  ListFiles(".\\",filename + ".no*.trace.extab",listf,false,true,false);
			if ((!r)||(listf.size() < 2)) {out << "Pas de fichiers multiple trouvés (" << filename <<  ".no*.trace.extab)\n"; return;}
			else 
				{
				out << "reading " << listf[0] << "\n";				
				ExTab me(listf[0]) ;
				for(int i=1;i<(int)listf.size();i++)
					{
					out << "reading " << listf[i] << "\n";
					ExTab mp(listf[i]); me+=mp;
					}
				me /= ((double)listf.size());
				outf1 = filename + ".average" + tostring(listf.size()) + ".trace.extab";
				me.Save(outf1);
				out << "\n" << listf.size() << " files founds.\n\n";
				}
			out << "file saved for the average time of increase of range\n  -> " << outf1 << "\n\n";
			}
			{
			std::vector<std::string> listf;
			bool r =  ListFiles(".\\",filename + ".no*.return.extab",listf,false,true,false);
			if ((!r)||(listf.size() < 2)) {out << "Pas de fichiers multiple trouvés (" << filename <<  ".no*.return.extab)\n"; return;}
			else 
				{
				out << "reading " << listf[0] << "\n";				
				ExTab me(listf[0]) ;
				for(int i=1;i<(int)listf.size();i++)
					{
					out << "reading " << listf[i] << "\n";
					ExTab mp(listf[i]); me+=mp;
					}
				me /= ((double)listf.size());
				outf2 = filename + ".average" + tostring(listf.size()) + ".return.extab";
				me.Save(outf2);
				out << "\n" << listf.size() << " files founds.\n\n";
				}
			out << "file saved for the average number of returns w.r.t. the range\n  -> " << outf2 << "\n\n";
			}
		out << "\n***** PACK FUSION COMPLETED *****\n\n";
		return;
		}



		void simulationOERRW::simulationMenu()
		{
		out << "********************************************************\n";
		out << "*   Once Edge Reinforced Random Walk simulation menu   *";
		out << "********************************************************\n";
		while(1)
			{
			out << "\n\nQue voulez vous faire ?\n\n";
			out << "1) Creer/continuer une simulation detaillee d'une OERRW\n";
			out << "2) Creer/continuer une simulation longue d'une OERRW\n";
			out << "3) Creer/continuer un pack de simulation simples\n";
			out << "4) afficher une simulation\n";
			out << "5) Fusionner les extabs d'un pack de simulation\n\n";
			int k = input.getkey();
			if (k == '4') 
				{
				std::string filename; std::string path = ".\\";
				out << "-> 4 - Affichage d'une simulation.\n";
				out << "nom du fichier .onceERRW (sans extension) ? ";	input >> filename; out << filename << "\n\n";
				printSimulation(".\\",filename);
				return;
				}
			if (k == '2') 
				{
				double d;
				out << "-> 2 - Simulation OERRW de longue durée.\n";
				out << "Valeur de delta ? "; input >> d; out << d << "\n\n";
				longSimulation(d,1000     ,10000);
				longSimulation(d,10000    ,100000);
				longSimulation(d,50000    ,500000);
				longSimulation(d,100000   ,1000000);
				longSimulation(d,500000   ,5000000);
				longSimulation(d,1000000  ,10000000);
				longSimulation(d,5000000  ,50000000);
				longSimulation(d,10000000 ,100000000);
				longSimulation(d,50000000 ,500000000);
				longSimulation(d,100000000,1000000000);
				longSimulation(d,200000000,2000000000);
				longSimulation(d,400000000,4000000000);
				longSimulation(d,800000000,8000000000);
				longSimulation(d,1000000000,-1);
				}
			if (k == '1')
				{
				double d;
				out << "-> 1 - Simulation detaillee d'une OERRW\n";
				out << "Valeur de delta ? "; input >> d; out << d << "\n\n";
				out	<< "Number of site to visit (in million) ? "; int64 end = 500; input >> end; out << end << "\n"; end *= 1000000;
				out << "interval between each save (in million) ? "; int sav = 50; input >> sav; out << sav << "\n\n"; sav *= 1000000;
				detailledSimulation(d,sav,end);
				return;
				}
			if (k=='3') 
				{
				double d;
				out << "-> 3 - Pack de simulations\n";
				out << "Valeur de delta ? "; input >> d; out << d << "\n\n";
				out << "Number of site to visit (in million) ? "; int64 end = 100; input >> end; out << end << "\n\n"; end *= 1000000;
				packSimulation(d,end);
				return;
				}
			if (k=='5') 
				{
				std::string outf1,outf2;
				double d;
				out << "-> 5 - Fusion des extab d'un pack de simulation\n";
				out << "Valeur de delta ? "; input >> d; out << d << "\n\n";
				out << "Number of site visited (in million) ? "; int64 end = 100; input >> end; out << end << "\n\n"; end *= 1000000;
				fusionPack(d,end);
				return;
				}
			}
		}
 


}

}


/* end of file onceERRW.cpp */

