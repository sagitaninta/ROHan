#include <iostream>
#include <fstream>
#include <queue>
#include <algorithm>   
#include <cfloat>   
//#include <random>

//TODO
// add deamination in calculation
// add mappability
//

#include "api/internal/io/BgzfStream_p.h"
#include <api/BamConstants.h>
#include <api/BamMultiReader.h>
#include <utils/bamtools_fasta.h>
#include <utils/bamtools_options.h>
#include <utils/bamtools_pileup_engine.h>
#include <utils/bamtools_utilities.h>

// #include "api/BamMultiReader.h"
// #include "api/BamReader.h"
// #include "api/BamWriter.h"
// #include "api/BamAux.h"

#include "GenomicWindows.h"

#include "miscfunc.h"
#include "utils.h"

using namespace std;
using namespace BamTools;

//#define DEBUGCOMPUTELL
#define HETVERBOSE
// #define COVERAGETVERBOSE

#define MAXMAPPINGQUAL 257     // maximal mapping quality, should be sufficient as mapping qualities are encoded using 8 bits
#define MAXCOV          50     // maximal coverage

char offsetQual=33;
long double likeMatch        [MAXMAPPINGQUAL];
long double likeMismatch     [MAXMAPPINGQUAL];

long double likeMatchProb    [MAXMAPPINGQUAL];
long double likeMismatchProb [MAXMAPPINGQUAL];

vector< vector<long double> > binomVec (MAXCOV+1,vector<long double>(MAXCOV+1,0)) ;
unsigned int totalBasesSum;
unsigned int totalSitesSum;
vector<substitutionRates> sub5p;
vector<substitutionRates> sub3p;
substitutionRates defaultSubMatch;
long double contrate=0.0;
long double rateForPoissonCov;
long double pdfRateForPoissonCov;

// // Returns logl( expl(x)+expl(y) )
// inline long double oplusl(long double x, long double y ){
//     return x > y 
//         ? x + log1pl( expl( y-x ) )
//         : y + log1pl( expl( x-y ) )  ;
// }




//TODO: put different classes and functions in different files
class DataChunk{
private:
    
    public:
    //vector<BamAlignment>  dataToProcess;    
    GenomicRange rangeGen;
    int rank;
    
    DataChunk();
    DataChunk(const DataChunk & other);
    ~DataChunk();
    DataChunk & operator= (const DataChunk & other);
};

DataChunk::DataChunk(){
    //cerr<<"Constructor addr: "<<this<<endl;
}

DataChunk::~DataChunk(){
    //cerr<<"Destructor  addr: "<<this<<endl;
}

class CompareDataChunk {
public:
    bool operator() ( DataChunk * cd1, DataChunk * cd2)  {
        //comparison code here
	return ( cd1->rank > cd2->rank );
    }
};





class PositionResult{
private:
    
    public:

    int          refID;
    unsigned int pos ;
    char         refB;
    char         altB;
    int          refC;
    int          altC;

    long double  rrll;
    long double  rall;
    long double  aall;

    long double  lqual;
    long double  llCov;
    int          geno;
    
    PositionResult();
    PositionResult(const PositionResult & other);
    ~PositionResult();
    string toString(const RefVector  references) const;

    PositionResult & operator= (const PositionResult & other);
    //    friend ostream & operator<<(ostream & os, const PositionResult & ct);
};



PositionResult::PositionResult(){
    //cerr<<"Constructor addr: "<<this<<endl;
}

PositionResult::~PositionResult(){
    //cerr<<"Destructor  addr: "<<this<<endl;
}




class GenoResults{
private:
    
public:

    long double  rrll;
    long double  rall;
    long double  aall;

    long double  lqual;
    long double  llCov;
    int          geno;
    
    GenoResults();
    GenoResults(long double  rrll ,
		long double  rall ,
		long double  aall ,
		long double  lqual,
		long double  llCov,
		int          geno);
    GenoResults(const PositionResult * pr);
    GenoResults(const GenoResults    & other);
    ~GenoResults();
    GenoResults & operator= (const GenoResults & other);
};

GenoResults::GenoResults(){
    //cerr<<"Constructor addr: "<<this<<endl;
}

GenoResults::GenoResults(const PositionResult * pr){
    rrll   = pr->rrll ;
    rall   = pr->rall ;
    aall   = pr->aall ;
    lqual  = pr->lqual;
    llCov  = pr->llCov;
    geno   = pr->geno ;    
}

GenoResults::GenoResults(long double  rrll ,
			 long double  rall ,
			 long double  aall ,
			 long double  lqual,
			 long double  llCov,
			 int          geno): 
    rrll(  rrll  ),
    rall(  rall  ),
    aall(  aall  ),
    lqual( lqual ),
    llCov( llCov ),
    geno(  geno  ){
    
}

GenoResults::~GenoResults(){
    //cerr<<"Destructor  addr: "<<this<<endl;
}


string PositionResult::toString(const RefVector  references) const{
    //cerr<<"Constructor addr: "<<this<<endl;
    string toReturn="";
    toReturn += ""+references[refID].RefName+"\t";
    toReturn += ""+stringify(pos)+"\t";

    toReturn += ""+stringify(refB)+"\t";
    toReturn += ""+stringify(altB)+"\t";

    toReturn += ""+stringify(refC)+"\t";
    toReturn += ""+stringify(altC)+"\t";

    if(geno==0){
	toReturn += "0/0\t";
    }else{
	if(geno==1){
	    toReturn += "0/1\t";
	}else{
	    if(geno==2){
		toReturn += "1/1\t";
	    }else{
		cerr<<"Internal error for genotype"<<endl;
		exit(1);
	    }	    
	}
    }


    toReturn += ""+stringify(rrll)+"\t";
    toReturn += ""+stringify(rall)+"\t";
    toReturn += ""+stringify(aall)+"\t";

    toReturn += ""+stringify(lqual)+"\t";
    toReturn += ""+stringify(llCov)+"\t";

    toReturn += "\n";

    //cout<<"toString "<<toReturn<<endl;
    return toReturn;
}

//TODO code to VCF
// string PositionResult::toString(const RefVector  references) const{
//     //cerr<<"Constructor addr: "<<this<<endl;
//     string toReturn="";
//     toReturn += ""+references[refID].RefName+"\t";
//     toReturn += ""+stringify(pos)+"\t";
//     toReturn += ""+stringify(refB)+"\t";
//     toReturn += ""+stringify(altB)+"\t";
//     toReturn += ".\t"; //ID
//     toReturn += "0\t"; //QUAL   
//     toReturn += "\t";

//     toReturn += "\n";

//     //cout<<"toString "<<toReturn<<endl;
//     return toReturn;
// }

// ostream & operator << (ostream & os, const PositionResult & pr){


//     os<<pr.toString();


//     return os;
// }






class DataToWrite{
private:
    
    public:
    vector<PositionResult *>  * vecPositionResults;
    GenomicRange rangeGen;
    int rank;
    
    DataToWrite();
    DataToWrite(const DataToWrite & other);
    ~DataToWrite();
    DataToWrite & operator= (const DataToWrite & other);
};

DataToWrite::DataToWrite(){
    vecPositionResults =  new vector<PositionResult *>();
    //cerr<<"Constructor addr: "<<this<<endl;
}

DataToWrite::~DataToWrite(){
    for (unsigned int i =0; i< vecPositionResults->size();i++){
	delete (vecPositionResults->at(i));
    } 
    vecPositionResults->clear();
    delete vecPositionResults;
    //cerr<<"Destructor  addr: "<<this<<endl;
}

class CompareDataToWrite {
public:
    bool operator() ( DataToWrite * cd1, DataToWrite * cd2)  {
        //comparison code here
	return ( cd1->rank > cd2->rank );
    }
};



int    timeThreadSleep =    10;
int    timeSleepWrite  =    1;

bool      readDataDone = false;
unsigned int sizeChunk =  5000;

string                                                             bamFileToOpen;
queue< DataChunk * >                                               queueDataToprocess;
queue< DataChunk * >                                               queueDataForCoverage;

priority_queue<DataToWrite *, vector<DataToWrite *>, CompareDataToWrite> queueDataTowrite;

pthread_mutex_t  mutexQueue   = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  mutexCounter = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  mutexRank    = PTHREAD_MUTEX_INITIALIZER;

//GLOBALLY accessed
map<unsigned int, int>       threadID2Rank;


//! Chunk of code to check if a certain thread call failed
/*!
  This block is calls by the pthread

*/				
#define checkResults(string, val) {             \
 if (val) {                                     \
     cerr<<"Failed with "<<val<<" at "<<string<<endl;	\
   exit(1);                                     \
 }                                              \
}
 


// typedef struct{
//     long double ll;
//     long double expal1; //expectation of # of allele 1
//     long double expal2; //expectation of # of allele 2

// } computeLLRes;


long double pdfPoisson(const long double l,const long double k ) {
    return expl(k*logl(l)-lgammal(k+1.0)-l);
}


//! A method to initialize various probability scores to avoid recomputation
/*!
  This method is called by the main after capturing the arguments
*/
void initScores(){
    totalBasesSum=0;
    totalSitesSum=0;

    //Computing for quality scores 2 and up
    for(int i=0;i<MAXMAPPINGQUAL;i++){
        likeMatch[i]        = log1pl(    -pow(10.0,i/-10.0) );          
        likeMismatch[i]     = logl  (     pow(10.0,i/-10.0) );

        likeMatchProb[i]           = 1.0-pow(10.0,i/-10.0);
        likeMismatchProb[i]        =     pow(10.0,i/-10.0);
    }



    for(int i=1;i<=MAXCOV;i++){
	//cout<<i<<endl;

	for(int j=0;j<=i;j++){	    
	    binomVec[i][j] = ( logl(nChoosek(i,j))+logl(powl(0.5,i)) );	     
	}
    }

}//end initScores


long double computeLL(const int                   al1Current,
		      const int                   al2Current,		      
		      const vector<int>         & obsBase   ,
		      const vector<long double> & probDeam  ,
		      const vector<int>         & obsQual   ,
		      const long double           contRate  ,
		      const int                   alContCurrent ,
		      const vector<long double> & mismappingProb
		      ){
    //computeLLRes toreturn;
    long double llik=0;
    long double llik1=0;
    long double llik2=0;
    long double llikC=0;
    
#ifdef DEBUGCOMPUTELL
    cout<<al1Current<<"\t"<<al2Current<<endl;
#endif

    for(int i=0;i<int(obsBase.size());i++){

	//contaminant
	if(obsBase[i] == alContCurrent){
	    llikC    =      (1.0-mismappingProb[i])*(likeMatchProb[obsQual[i]]*(1.0) + likeMismatchProb[obsQual[i]]*(0.5))+mismappingProb[i]*0.5;
	}else{
	    llikC    =      (1.0-mismappingProb[i])*(likeMatchProb[obsQual[i]]*(0.0) + likeMismatchProb[obsQual[i]]*(0.5))+mismappingProb[i]*0.5;
	}

	long double llikAl1t=0;
	long double llikAl2t=0;

	//replace prob deam with proper value from matrix
	if(obsBase[i] == al1Current){
	    llikAl1t =     (1.0-mismappingProb[i])*(likeMatchProb[obsQual[i]]*(1.0-probDeam[i])  + likeMismatchProb[obsQual[i]]*(0.5))+mismappingProb[i]*0.5;;
	}else{
	    llikAl1t =     (1.0-mismappingProb[i])*(likeMatchProb[obsQual[i]]*(    probDeam[i])  + likeMismatchProb[obsQual[i]]*(0.5))+mismappingProb[i]*0.5;
	}


	if(obsBase[i] == al2Current){
	    llikAl2t =     (1.0-mismappingProb[i])*(likeMatchProb[obsQual[i]]*(1.0-probDeam[i])  + likeMismatchProb[obsQual[i]]*(0.5))+mismappingProb[i]*0.5;;
	}else{
	    llikAl2t =     (1.0-mismappingProb[i])*(likeMatchProb[obsQual[i]]*(    probDeam[i])  + likeMismatchProb[obsQual[i]]*(0.5))+mismappingProb[i]*0.5;
	}

#ifdef DEBUGCOMPUTELL
	cout<<i<<obsBase[i]<<"\t"<<al1Current<<"\t"<<al2Current<<"\t"<<alContCurrent<<endl;
	cout<<i<<"\t"<<likeMatchProb[obsQual[i]]<<"\t"<<likeMismatchProb[obsQual[i]]<<"\t"<<mismappingProb[i]<<endl;
	cout<<i<<"\t"<<llikAl1t <<"\t"<<llikAl2t<<endl;
#endif
	// exit(1);
	long double llikT  = (1.0-contRate)*( 0.5*llikAl1t + 0.5*llikAl2t ) + (contRate)*llikC  ;	
	llik              += logl(llikT);

 	llik1=oplusInitnatl( llik1, logl(llikAl1t) );
	llik2=oplusInitnatl( llik2, logl(llikAl2t) );

#ifdef DEBUGCOMPUTELL
	cout<<i<<"\t"<<llikAl1t <<"\t"<<llikAl2t<<"\t"<<llikC<<"\t"<<llikT<<endl;
#endif
	// llik1+=logl( llikAl1t );
	// llik2+=logl( llikAl2t );
    }
	//cout<<"CC\t"<<llikCC<<"\t"<<llikCC1<<"\t"<<llikCC2<<endl;    
#ifdef DEBUGCOMPUTELL
   cout<<llik1<<"\t"<<llik2<<"\t"<<expl(llik1)<<"\t"<<expl(llik2)<<endl;
#endif

    long double expal1  = roundl(  int(obsBase.size()) * ( expl(llik1) / expl(oplusnatl(llik1,llik2))) );
    //long double expal1=roundl(  sizeAr * ( expl(llik1) / llik1+llik2)) );
    //long double expal2  = sizeAr-expal1;
    long double binomE2 = binomVec[int(obsBase.size())][expal1]; //logl(nChoosek(sizeAr,expal1)*powl(0.5,expal1+expal2));
    
#ifdef DEBUGCOMPUTELL
    cout<<al1Current<<""<<al2Current<<"\t"<<llik<<"\t"<<expal1<<"\t"<<(int(obsBase.size())-expal1)<<endl;
    cout<<binomE2<<"\t"<<(binomE2+llik)<<endl;
#endif
    
    // toreturn.ll     = (binomE2+llik);
    // toreturn.expal1 = expal1;
    // toreturn.expal2 = expal2;
    
    return (binomE2+llik);
} //end computeLL


class coverageComputeVisitor : public PileupVisitor {
  
public:
    coverageComputeVisitor(const RefVector& references,unsigned int leftCoord, unsigned int rightCoord)
	: PileupVisitor()
	, m_references(references)
	, m_leftCoord(leftCoord)
	, m_rightCoord(rightCoord)
    { 
	totalBases=0;
	totalSites=0;
	
    }
    ~coverageComputeVisitor(void) {}
  
    // PileupVisitor interface implementation

    
    void Visit(const PileupPosition& pileupData) {   
	//bool foundOneFragment=false;
	if(pileupData.Position < int(m_leftCoord)   || 
	   pileupData.Position > int(m_rightCoord) ){
	    return ;
	}
	//cout<<m_leftCoord<<"\t"<<m_rightCoord<<"\t"<<pileupData.Position<<endl;

	for(unsigned int i=0;i<pileupData.PileupAlignments.size();i++){
	    if( pileupData.PileupAlignments[i].IsCurrentDeletion &&
	    	pileupData.PileupAlignments[i].IsNextInsertion ){
	    	continue;
	    }
	    //foundOneFragment=true;		  
	    totalBases++;
	}

	//if(foundOneFragment)
	totalSites++;
    }
    
    unsigned int getTotalBases() const{
    	return totalBases;
    }

    unsigned int getTotalSites() const{
    	return totalSites;
    }

private:
    RefVector m_references;
    //Fasta * m_fastaReference;
    unsigned int totalBases;
    unsigned int totalSites;
    unsigned int m_leftCoord;
    unsigned int m_rightCoord;
    
};//end coverageComputeVisitor




class heteroComputerVisitor : public PileupVisitor {
  
public:
    heteroComputerVisitor(const RefVector& references, 
			  const int refID,
			  const unsigned int leftCoord,
			  const unsigned int rightCoord,
			  const long double contRate,
			  vector<PositionResult *> * dataToWriteOut)
	: PileupVisitor()
	, m_references(references)
	, m_refID(refID)
	, m_leftCoord(leftCoord)
	, m_rightCoord(rightCoord)
	, m_contRate(contRate)
	, m_dataToWriteOut( dataToWriteOut)
    { 
    }
    ~heteroComputerVisitor(void) { }
  
    // PileupVisitor interface implementation

    
    void Visit(const PileupPosition& pileupData) {   


	if(pileupData.Position < int(m_leftCoord)   || 
	   pileupData.Position > int(m_rightCoord) ){
	    return ;
	}

	int                 totalBases=0;
	int                 counterB  [4];
	long double         llBaseDeam[4];
	vector<int>         obsBase   ;
	vector<int>         obsQual   ;
	vector<long double> probDeam  ;
	vector<long double> mmProb    ;
	unsigned int posAlign = pileupData.Position+1;

	for(unsigned int i=0;i<4;i++){
	    counterB[i]   = 0;
	    llBaseDeam[i] = 0.0;
	}

	for(unsigned int i=0;i<pileupData.PileupAlignments.size();i++){
	    if( pileupData.PileupAlignments[i].IsCurrentDeletion &&
	    	pileupData.PileupAlignments[i].IsNextInsertion ){
	    	continue;
	    }


	    if(i>=MAXCOV){
		break;
	    }

	    char  b   =     pileupData.PileupAlignments[i].Alignment.QueryBases[ pileupData.PileupAlignments[i].PositionInAlignment ];
	    if(!isResolvedDNA(b)){ 
		continue; 
	    }//avoid Ns
	    int bIndex = baseResolved2int(b);
	    int   q   = int(pileupData.PileupAlignments[i].Alignment.Qualities[  pileupData.PileupAlignments[i].PositionInAlignment ]-offsetQual); 
	    int   m   = int(pileupData.PileupAlignments[i].Alignment.MapQuality);
	  

	    // BEGIN DEAMINATION COMPUTATION
            //zero base distance to the 5p/3p end
            int dist5p=-1;
            int dist3p=-1;

	    bool isRev = pileupData.PileupAlignments[i].Alignment.IsReverseStrand();
            if( isRev ){
                dist5p = pileupData.PileupAlignments[i].Alignment.QueryBases.size() - pileupData.PileupAlignments[i].PositionInAlignment-1;
                dist3p = pileupData.PileupAlignments[i].PositionInAlignment;
            }else{
                dist5p = pileupData.PileupAlignments[i].PositionInAlignment;
                dist3p = pileupData.PileupAlignments[i].Alignment.QueryBases.size() - pileupData.PileupAlignments[i].PositionInAlignment-1;
            }
                                    

            // probSubstition * probSubMatchToUseEndo = &defaultSubMatch ;
            // probSubstition * probSubMatchToUseCont = &defaultSubMatch ;
            substitutionRates * probSubMatchToUseEndo = &defaultSubMatch ;
            // substitutionRates * probSubMatchToUseCont = &defaultSubMatch ;

            if(dist5p <= (int(sub5p.size()) -1)){
                probSubMatchToUseEndo = &sub5p[  dist5p ];                      
            }

            if(dist3p <= (int(sub3p.size()) -1)){
                probSubMatchToUseEndo = &sub3p[  dist3p ];
            }
            //we have substitution probabilities for both... take the closest
            if(dist5p <= (int(sub5p.size()) -1) &&
               dist3p <= (int(sub3p.size()) -1) ){
                    
                if(dist5p < dist3p){
                    probSubMatchToUseEndo = &sub5p[  dist5p ];
                }else{
                    probSubMatchToUseEndo = &sub3p[  dist3p ];
                }
                    
            }
	    
	    //cout<<"deamdist\t"<<dist5p<<"\t"<<dist3p<<endl;
            //we look for a damage going from bIndexAlt to bIndex
	    for(int bIndexAlt=0;bIndexAlt<4;bIndexAlt++){
		if(bIndex==bIndexAlt) continue;
		
		int dinucIndex;
		if( isRev ){		    
		    dinucIndex =     dimer2indexInt(complementInt(bIndexAlt),complementInt(bIndex));
		}else{
		    dinucIndex =     dimer2indexInt(              bIndexAlt ,              bIndex);
		}
                                                                         
		long double probSubDeam              = probSubMatchToUseEndo->s[dinucIndex];
		long double probSameDeam             = 1.0-probSubDeam;
		//long double probSameDeam           = probSubMatch->s[dinucIndex
		//cout<<"deam\t"<<bIndex<<"\t"<<bIndexAlt<<"\t"<<probSubDeam<<"\t"<<probSameDeam<<endl;
		llBaseDeam[bIndexAlt] += logl(probSameDeam);
	    }
	    // END DEAMINATION COMPUTATION


  
	    //cout<<"pos "<<posAlign<<" "<<bIndex<<" "<<b<<endl;
	    counterB[ bIndex ]++;
	    totalBases++;
	    obsBase.push_back(             bIndex   );
	    obsQual.push_back(                  q   );
	    mmProb.push_back(  likeMismatchProb[m]  );
	    
	    //TODO: Fill deamination vector, do not forget fragment orientation
	    //put proper probabilities
	    probDeam.push_back( 0.0   );
	}//end for each read
	
	long double nonZerollBaseDeam  = 0;
	int         nonZerollBaseDeamI = -1;

	for(int i=0;i<4;i++){
	    //cout<<"deamres\t"<<i<<"\t"<<llBaseDeam[i]<<endl;
	    if(llBaseDeam[i] !=0){
		if(  nonZerollBaseDeam > llBaseDeam[i] ){
		    nonZerollBaseDeam  = llBaseDeam[i];
		    nonZerollBaseDeamI =            i;		    
		}
	    }
	}

	// if(nonZerollBaseDeamI!=-1){
	//     cout<<"nonzero\t"<<nonZerollBaseDeam<<"\t"<<nonZerollBaseDeamI<<endl;
	//     //exit(1);
	// }


	int counterUnique=0;
	for(int i=0;i<4;i++){
	    if(counterB[i]!=0) 
		counterUnique++;
	}

	// cout<<"pos "<<posAlign<<" unique "<<counterUnique<<endl;

	//skip sites with no defined bases and tri/tetra allelic sites
	if(counterUnique==0 ||
	   counterUnique>=3){
	    return;
	}

	int ref=-1; //not really reference base, just the majority base
	int alt=-1;

	if(counterUnique==1){
	    for(int i=0;i<4;i++){
		if(counterB[i]!=0) ref=i;
	    }
	    
	    if(nonZerollBaseDeamI!=-1){
		alt=nonZerollBaseDeamI;
	    }else{
		alt=randomBPExceptInt(ref);	    //todo maybe put a better dna sub model here?
	    }
	}

	//TODO: decide if   if(nonZerollBaseDeamI!=-1){ do we dump the site?
	if(counterUnique==2){
	    for(int i=0;i<4;i++){
		if(counterB[i]!=0){
		    if(ref == -1){
			ref=i;
		    }else{
			if(counterB[i] < counterB[ref] ){//more ref bases than i
			    alt = i;
			}else{//more alt bases than i, alt becomes ref
			    alt = ref;
			    ref = i;
			}
		    }
		}
	    }
	}
	

	
	char refB="ACGT"[ref];
	char altB="ACGT"[alt];
	PositionResult * prToAdd=new PositionResult();
	prToAdd->refID = m_refID;
	prToAdd->pos   = posAlign;
	prToAdd->refB  = refB;
	prToAdd->altB  = altB;
	prToAdd->refC  = counterB[ref];
	prToAdd->altC  = counterB[alt];



	//cout<<posAlign<<"\t"<<refB<<","<<altB<<"\t"<<counterB[ref]<<"\t"<<counterB[alt]<<endl;
	long double rrllCr=computeLL(ref         ,
				     ref         ,		      		  
				     obsBase     ,
				     probDeam    ,
				     obsQual     ,
				     m_contRate  ,
				     ref         ,
				     mmProb      );
	long double rallCr=computeLL(ref         ,
				     alt         ,		      		  
				     obsBase     ,
				     probDeam    ,
				     obsQual     ,
				     m_contRate  ,
				     ref         ,
				     mmProb      );
       long double aallCr=computeLL(alt          ,
				    alt          ,		      		  
				    obsBase      ,
				    probDeam     ,
				    obsQual      ,
				    m_contRate   ,
				    ref          ,
				    mmProb       );


       long double rrllCa=0;
       long double rallCa=0;
       long double aallCa=0;
       
       if(m_contRate>0){

	   rrllCa=computeLL(ref          ,
			    ref          ,		      		  
			    obsBase      ,
			    probDeam     ,
			    obsQual      ,
			    m_contRate   ,
			    alt          ,
			    mmProb       );

	   rallCa=computeLL(ref          ,
			    alt          ,		      		  
			    obsBase      ,
			    probDeam     ,
			    obsQual      ,
			    m_contRate   ,
			    alt          ,
			    mmProb       );

	   aallCa=computeLL(alt          ,
			    alt          ,		      		  
			    obsBase      ,
			    probDeam     ,
			    obsQual      ,
			    m_contRate   ,
			    alt          ,
			    mmProb       );

       }else{
	   rrllCa=rrllCr;
	   rallCa=rallCr;
	   aallCa=aallCr;
       }
       
       prToAdd->rrll  = oplusnatl( rrllCr+logl(0.5), rrllCa+logl(0.5));
       prToAdd->rall  = oplusnatl( rallCr+logl(0.5), rallCa+logl(0.5));
       prToAdd->aall  = oplusnatl( aallCr+logl(0.5), aallCa+logl(0.5));
       vector<long double> arrLL (3,0); 
       arrLL[0]       = prToAdd->rrll;
       arrLL[1]       = prToAdd->rall;
       arrLL[2]       = prToAdd->aall;
       sort (arrLL.begin(), arrLL.end());
       prToAdd->lqual = (arrLL[2]-arrLL[1]);
       //1st most likely = 2
       //2nd most likely = 1

       if(arrLL[2]     == prToAdd->rrll){
	   prToAdd->geno = 0;     //rr
       }else{
	   if(arrLL[2] == prToAdd->rall){
	       prToAdd->geno = 1; //ra
	   }else{
	       prToAdd->geno = 2; //aa
	   }
       }

       prToAdd->llCov = logl( pdfPoisson( (long double)totalBases, rateForPoissonCov)/pdfRateForPoissonCov );

       m_dataToWriteOut->push_back(prToAdd);
       
    }//end Visit()
    

private:
    RefVector m_references;
    //Fasta * m_fastaReference;
    // unsigned int totalBases;
    // unsigned int totalSites;
    int          m_refID;
    unsigned int m_leftCoord;
    unsigned int m_rightCoord;
    long double  m_contRate;
    vector<PositionResult *> * m_dataToWriteOut;
};//heteroComputerVisitor










//! Method called for each thread
/*!
  

*/				
void *mainHeteroComputationThread(void * argc){

    int   rc;
#ifdef HETVERBOSE    
    int rankThread=0;
#endif
    rc = pthread_mutex_lock(&mutexRank);
    checkResults("pthread_mutex_lock()\n", rc);

    threadID2Rank[*(int *)pthread_self()]  = threadID2Rank.size()+1;

#ifdef HETVERBOSE    
    rankThread = threadID2Rank[*(int *)pthread_self()];
#endif
    
    rc = pthread_mutex_unlock(&mutexRank);
    checkResults("pthread_mutex_unlock()\n", rc);

 checkqueue:    
    // stackIndex=-1;
    //check stack

    
    rc = pthread_mutex_lock(&mutexQueue);
    checkResults("pthread_mutex_lock()\n", rc);


    bool foundData=false;
    
#ifdef HETVERBOSE
    cerr<<"Thread #"<<rankThread <<" started and is requesting data"<<endl;
#endif

    DataChunk    * currentChunk;


    if(!queueDataToprocess.empty()){    
 	foundData=true;
 	currentChunk = queueDataToprocess.front();
 	queueDataToprocess.pop();
#ifdef HETVERBOSE
 	cerr<<"Thread #"<<rankThread<<" is reading "<<currentChunk->rank<<endl;
#endif
	//cout<<"rank "<< &(currentChunk->dataToProcess) <<endl;
    }

    
  

    if(!foundData){
	rc = pthread_mutex_unlock(&mutexQueue);
	checkResults("pthread_mutex_unlock()\n", rc);


	if(readDataDone){
#ifdef HETVERBOSE
	    cerr<<"Thread #"<<rankThread<<" is done"<<endl;
#endif
	    return NULL;	
	}else{
#ifdef HETVERBOSE
	    cerr<<"Thread #"<<rankThread<<" sleeping for "<<timeThreadSleep<<endl;
#endif
	    sleep(timeThreadSleep);
	    goto checkqueue;
	}

    }else{
	//release stack
	rc = pthread_mutex_unlock(&mutexQueue);
	checkResults("pthread_mutex_unlock()\n", rc);
    }

    //////////////////////////////////////////////////////////////
    //                BEGIN COMPUTATION                         //
    //////////////////////////////////////////////////////////////

    //cout<<currentChunk->rangeGen<<endl;

#ifdef HETVERBOSE
    cerr<<"Thread #"<<rankThread<<" is reading "<<currentChunk->rangeGen<<endl;
#endif

    //sleep(10);


    


    BamReader reader;
    if ( !reader.Open(bamFileToOpen) ) {
	cerr << "Could not open input BAM file:" << bamFileToOpen <<endl;
    	exit(1);
    }

    reader.LocateIndex();

    if(!reader.HasIndex()){
    	cerr << "The BAM file: " << bamFileToOpen <<" does not have an index"<<endl;
    	exit(1);
    }

    // retrieve reference data
    const RefVector  references = reader.GetReferenceData();
    const int        refID      = reader.GetReferenceID( currentChunk->rangeGen.getChrName() );
    
    // cerr<<"Thread #"<<rankThread<<" refID "<<refID<<endl;    
    // cerr<<"Thread #"<<rankThread<<" "<<references[0].RefName<<endl;

    BamRegion bregion (refID, 
		       currentChunk->rangeGen.getStartCoord(), 
		       refID, 
		       currentChunk->rangeGen.getEndCoord()   );

    bool setRegionRes=reader.SetRegion( bregion   );

    // cerr<<"Thread #"<<rankThread<<" "<<references[0].RefName<<"\t"<<setRegionRes<<endl;

    if( refID==-1 ||
       !setRegionRes){
    	cerr << "Heterozygous computation: could not set region "<<currentChunk->rangeGen<<" for BAM file:" << bamFileToOpen <<" "<<currentChunk->rangeGen.getStartCoord()<<" "<< currentChunk->rangeGen.getEndCoord()<< endl;
    	exit(1);
    }

    DataToWrite  * dataToWrite = new DataToWrite();

    dataToWrite->rangeGen      =  currentChunk->rangeGen;
    dataToWrite->rank          =  currentChunk->rank;

    //dataToWrite->dataToWriteOut=new vector<PositionResult *>();
    heteroComputerVisitor* cv = new heteroComputerVisitor(references,
							  refID,
							  currentChunk->rangeGen.getStartCoord(), 
							  currentChunk->rangeGen.getEndCoord()  ,
							  contrate,
							  dataToWrite->vecPositionResults);

    

    PileupEngine pileup;
    pileup.AddVisitor(cv);

    BamAlignment al;
    //unsigned int numReads=0;
    while ( reader.GetNextAlignment(al) ) {
        pileup.AddAlignment(al);
    }

    //clean up
    pileup.Flush();
    reader.Close();
    //fastaReference.Close();
    
    //cerr<<"Thread #"<<rankThread <<" "<<cv->getTotalBases()<<"\t"<<cv->getTotalSites()<<"\t"<<double(cv->getTotalBases())/double(cv->getTotalSites())<<endl;

    delete cv;

	
#ifdef HETVERBOSE
    cerr<<"Thread #"<<rankThread <<" is done with computations"<<endl;
#endif

    //////////////////////////////////////////////////////////////
    //                END   COMPUTATION                         //
    //////////////////////////////////////////////////////////////
	

    //COUNTERS
    rc = pthread_mutex_lock(&mutexCounter);
    checkResults("pthread_mutex_lock()\n", rc);
    

    //TODO ADD QUEUE
    queueDataTowrite.push(dataToWrite);

    rc = pthread_mutex_unlock(&mutexCounter);
    checkResults("pthread_mutex_unlock()\n", rc);

#ifdef HETVERBOSE
    cerr<<"Thread #"<<rankThread <<" is re-starting"<<endl;
#endif
    goto checkqueue;	   


    

#ifdef HETVERBOSE    
    cerr<<"Thread "<<rankThread<<" ended "<<endl;
#endif

    return NULL;

}// end mainHeteroComputationThread






queue< DataChunk * >  randomSubQueue(const queue< DataChunk * > queueDataToSubsample,unsigned int sizeToReturn){

    if( sizeToReturn > queueDataToSubsample.size()){
	cerr<<"Cannot subsample the queue to the size required"<<endl;
	exit(1);
    }

    queue< DataChunk *  >  toReturn=queueDataToSubsample;
    vector< DataChunk * >  myvectortemp;

    while(!toReturn.empty()){    
	DataChunk *dc = toReturn.front();
 	toReturn.pop();
	myvectortemp.push_back(dc);
    }

    random_shuffle ( myvectortemp.begin(), myvectortemp.end() );
    
    for(unsigned int i=0;i<sizeToReturn;i++){
	toReturn.push(myvectortemp[i]);
    }

    return toReturn;
} // end randomSubQueue



void *mainCoverageComputationThread(void * argc){
    initScores();
    int   rc;
#ifdef COVERAGETVERBOSE    
    int rankThread=0;
#endif
    rc = pthread_mutex_lock(&mutexRank);
    checkResults("pthread_mutex_lock()\n", rc);

    threadID2Rank[*(int *)pthread_self()]  = threadID2Rank.size()+1;
#ifdef COVERAGETVERBOSE    
    rankThread = threadID2Rank[*(int *)pthread_self()];
#endif
    
    rc = pthread_mutex_unlock(&mutexRank);
    checkResults("pthread_mutex_unlock()\n", rc);

 checkqueue:    
    // stackIndex=-1;
    //check stack

    
    rc = pthread_mutex_lock(&mutexQueue);
    checkResults("pthread_mutex_lock()\n", rc);


    bool foundData=false;
#ifdef COVERAGETVERBOSE
    cerr<<"Thread coverage #"<<rankThread <<" started and is requesting data"<<endl;
#endif

    DataChunk * currentChunk;


    if(!queueDataForCoverage.empty()){    
 	foundData=true;
 	currentChunk = queueDataForCoverage.front();
 	queueDataForCoverage.pop();
#ifdef COVERAGETVERBOSE
 	cerr<<"Thread #"<<rankThread<<" is reading "<<currentChunk->rank<<endl;
#endif
	//cout<<"rank "<< &(currentChunk->dataToProcess) <<endl;
    }

    
  

    if(!foundData){
	rc = pthread_mutex_unlock(&mutexQueue);
	checkResults("pthread_mutex_unlock()\n", rc);


	if(readDataDone){
#ifdef COVERAGETVERBOSE
	    cerr<<"Thread #"<<rankThread<<" is done"<<endl;
#endif
	    return NULL;	
	}else{
#ifdef COVERAGETVERBOSE
	    cerr<<"Thread #"<<rankThread<<" sleeping for "<<timeThreadSleep<<endl;
#endif
	    sleep(timeThreadSleep);
	    goto checkqueue;
	}

    }else{
	//release stack
	rc = pthread_mutex_unlock(&mutexQueue);
	checkResults("pthread_mutex_unlock()\n", rc);
    }

    //////////////////////////////////////////////////////////////
    //                BEGIN COMPUTATION                         //
    //////////////////////////////////////////////////////////////

    //cout<<currentChunk->rangeGen<<endl;
#ifdef COVERAGETVERBOSE
    cerr<<"Thread #"<<rankThread<<" is reading "<<currentChunk->rangeGen<<endl;
#endif
    //sleep(10);


    


    BamReader reader;
    if ( !reader.Open(bamFileToOpen) ) {
	cerr << "Could not open input BAM file:" << bamFileToOpen <<endl;
    	exit(1);
    }

    reader.LocateIndex();

    if(!reader.HasIndex()){
    	cerr << "The BAM file: " << bamFileToOpen <<" does not have an index"<<endl;
    	exit(1);
    }

    // retrieve reference data
    const RefVector  references = reader.GetReferenceData();
    const int        refID      = reader.GetReferenceID( currentChunk->rangeGen.getChrName() );


#ifdef COVERAGETVERBOSE    
    cerr<<"Thread #"<<rankThread<<" refID "<<refID<<" "<<currentChunk->rangeGen.getStartCoord()<<" "<<currentChunk->rangeGen.getEndCoord()<<endl;    
#endif
    // cerr<<"Thread #"<<rankThread<<" "<<references[0].RefName<<endl;


    BamRegion bregion (refID, 
    		       int(currentChunk->rangeGen.getStartCoord()), 
    		       refID, 
    		       int(currentChunk->rangeGen.getEndCoord() )  );

    bool setRegionRes = reader.SetRegion( bregion   );


    if( refID==-1 ||
       !setRegionRes){	
    	cerr << "Coverage computation: could not set region "<<currentChunk->rangeGen<<" for BAM file:" << bamFileToOpen <<" "<<currentChunk->rangeGen.getStartCoord()<<" "<< currentChunk->rangeGen.getEndCoord()<< "\trefID:"<<refID<<"\tset region fail?:"<<booleanAsString(setRegionRes)<<endl;
    	exit(1);
    }

   
    coverageComputeVisitor* cv = new coverageComputeVisitor(references,
							    currentChunk->rangeGen.getStartCoord(),
							    currentChunk->rangeGen.getEndCoord());
    PileupEngine pileup;
    pileup.AddVisitor(cv);

    BamAlignment al;
    //unsigned int numReads=0;
    while ( reader.GetNextAlignment(al) ) {
        pileup.AddAlignment(al);
    }

    
    //clean up
    pileup.Flush();
    reader.Close();
    //fastaReference.Close();
#ifdef COVERAGETVERBOSE    
    cerr<<"Thread #"<<rankThread <<" "<<cv->getTotalBases()<<"\t"<<cv->getTotalSites()<<"\t"<<double(cv->getTotalBases())/double(cv->getTotalSites())<<endl;
#endif

    unsigned int totalBasesL=cv->getTotalBases();
    unsigned int totalSitesL=cv->getTotalSites();


    delete cv;
	
#ifdef COVERAGETVERBOSE    
    cerr<<"Thread #"<<rankThread <<" is done with computations"<<endl;
#endif
    //////////////////////////////////////////////////////////////
    //                END   COMPUTATION                         //
    //////////////////////////////////////////////////////////////
	

    //COUNTERS
    rc = pthread_mutex_lock(&mutexCounter);
    checkResults("pthread_mutex_lock()\n", rc);
    
//queueDataTowrite.push(currentChunk);
    totalBasesSum+=totalBasesL;
    totalSitesSum+=totalSitesL;

    rc = pthread_mutex_unlock(&mutexCounter);
    checkResults("pthread_mutex_unlock()\n", rc);

#ifdef COVERAGETVERBOSE    
    cerr<<"Thread #"<<rankThread <<" is re-starting"<<endl;
#endif

    goto checkqueue;	   


    

#ifdef COVERAGETVERBOSE        
    cerr<<"Thread "<<rankThread<<" ended "<<endl;
#endif

    return NULL;

} // mainCoverageComputationThread





//! Main method
/*!
  The main:
    calls initScores(), 
    captures the arguments
    reads the deamination profiles
*/

int main (int argc, char *argv[]) {
    setlocale(LC_ALL, "POSIX");

    ////////////////////////////////////
    // BEGIN Initializing scores      //
    ////////////////////////////////////
    initScores();
    ////////////////////////////////////
    //    END Initializing scores     //
    ////////////////////////////////////


    ////////////////////////////////////
    // BEGIN Parsing arguments        //
    ////////////////////////////////////

    string cwdProg=getCWD(argv[0]);    

    string deam5pfreqE = getFullPath(cwdProg+"../deaminationProfile/none.prof");
    string deam3pfreqE = getFullPath(cwdProg+"../deaminationProfile/none.prof");

    //no contaminant deamination for now
    // string deam5pfreqC = getCWD(argv[0])+"deaminationProfile/none.prof";
    // string deam3pfreqC = getCWD(argv[0])+"deaminationProfile/none.prof";

    vector<substitutionRates>    deam5PsubE;
    vector<substitutionRates>    deam3PsubE;
    // vector<substitutionRates>    deam5PsubC;
    // vector<substitutionRates>    deam3PsubC;


    int    numberOfThreads   = 1;
    string outFileSiteLL;
    bool   outFileSiteLLFlag=false;

    string sampleName        = "sample";
    bool   useVCFoutput      = false;


    const string usage=string("\nThis program will do something beautiful\n\n\t"+
                              string(argv[0])+                        
                              " [options] [fasta file] [bam file]  "+"\n\n"+
			      "\twhere:\n"+
			      "\t\t[fasta file]\t\tThe fasta file used for alignement\n"
			      "\t\t[bam file]\t\tThe aligned and indexed BAM file\n"+
			      "\n\n"
			      
                              "\n\tI/O options:\n"+
			      "\t\t"+"-o"+"\t"+"--out"  + "\t\t"   +    "[outfile]" +"\t\t"+"Output per-site likelihoods in BGZIP (default: none)"+"\n"+
			      "\t\t"+""  +"\t"+"--name" + "\t\t"   +    "[name]"    +"\t\t\t"+"Sample name (default: "+sampleName+")"+"\n"+
			      "\t\t"+""  +""+"--vcf"    + "\t\t\t" +    ""          +"\t\t\t"+"Use VCF as output format (default: "+booleanAsString(useVCFoutput)+")"+"\n"+
			      
			      "\n\tComputation options:\n"+
                              "\t\t"+"-t"+"\t"+""       +"\t\t"    +    "[threads]" +"\t\t"+"Number of threads to use (default: "+stringify(numberOfThreads)+")"+"\n"+
                              "\t\t"+""  +""+"--phred64"+"\t\t\t"  +    ""          +"\t\t"+"Use PHRED 64 as the offset for QC scores (default : PHRED33)"+"\n"+
			      "\t\t"+""  +""+"--size"       +"\t\t\t"    + "[window size]" +"\t\t"+"Size of windows in bp  (default: "+stringify(sizeChunk)+")"+"\n"+	      

                              "\n\tSample options:\n"+
                              "\t\t"+""  +""+"--cont"  +"\t\t\t"    +  "[cont rate:0-1]" +"\t\t"+"Present-day human contamination rate (default: "+stringify(contrate)+")"+"\n"+
                              // "\t\t"+"--phred64" +"\t\t\t\t"+"Use PHRED 64 as the offset for QC scores (default : PHRED33)"+"\n"+
			      
			      
                              "\n\tDeamination options:\n"+                                   
                              "\t\t"+""  +""+"--deam5p\t\t"+"[.prof file]" +"\t\t"+"5p deamination frequency for the endogenous\n\t\t\t\t\t\t\t\t(default: "+deam5pfreqE+")"+"\n"+
                              "\t\t"+""  +""+"--deam3p\t\t"+"[.prof file]" +"\t\t"+"3p deamination frequency for the endogenous\n\t\t\t\t\t\t\t\t(default: "+deam3pfreqE+")"+"\n"+
                              // "\t\t"+"-deam5pc [.prof file]" +"\t\t"+"5p deamination frequency for the contaminant (default: "+deam5pfreqC+")"+"\n"+
                              // "\t\t"+"-deam3pc [.prof file]" +"\t\t"+"3p deamination frequency for the contaminant (default: "+deam3pfreqC+")"+"\n"+
			      

"");


    if( (argc== 1) ||
        (argc== 2 && string(argv[1]) == "-h") ||
        (argc== 2 && string(argv[1]) == "-help") ||
        (argc== 2 && string(argv[1]) == "--help") ){
        cout<<usage<<endl;
        return 1;
    }

    int lastOpt=1;

    for(int i=1;i<(argc);i++){ 

        if(string(argv[i])[0] != '-'  ){
            lastOpt=i;
            break;
        }


	
        if( string(argv[i]) == "--size"  ){
	    sizeChunk=destringify<unsigned int>(argv[i+1]);
            continue;
        }
	
        if( string(argv[i]) == "--vcf"  ){
	    useVCFoutput=true;
            continue;
        }

        if( string(argv[i]) == "--name"  ){
            sampleName=string(argv[i+1]);
            i++;
            continue;
        }

        if( string(argv[i]) == "--cont"  ){
            contrate=destringify<long double>(argv[i+1]);
            i++;
            continue;
        }


        if( string(argv[i]) == "-o"    ||
	    string(argv[i]) == "--out" ){
            outFileSiteLL=string(argv[i+1]);
	    outFileSiteLLFlag=true;
            i++;
            continue;
        }


        if(string(argv[i]) == "--phred64"  ){
            offsetQual=64;
            continue;
        }

        if(string(argv[i]) == "--deam5p"  ){
            deam5pfreqE=string(argv[i+1]);
            i++;
            continue;
        }

        if(string(argv[i]) == "--deam3p"  ){
            deam3pfreqE=string(argv[i+1]);
            i++;
            continue;
        }


	cerr<<"Error: unknown option "<<string(argv[i])<<endl;
	return 1;
    }


    string fastaFile         = string(argv[lastOpt]);
    bamFileToOpen            = string(argv[lastOpt+1]);
    string fastaIndex        = fastaFile+".fai";

    if( !isFile(fastaFile) ){
	cerr<<"The fasta file "<<fastaFile<<" does not exists"<<endl;
	return 1;	
    }

    if( !isFile(fastaIndex) ){
	cerr<<"The fasta file "<<fastaFile<<"  does not have an index: "<<fastaIndex<<endl;
	return 1;	
    }

    if( contrate<0 || 
	contrate>1 ){
	cerr<<"The contamination rate must be between 0 and 1"<<endl;
	return 1;	
    }

    if(outFileSiteLLFlag)
	if( !strEndsWith(outFileSiteLL,".gz")){
	    cerr<<"The output file "<<outFileSiteLL<<" must end with .gz"<<endl;
	    return 1;	
	}




    //Testing BAM file
    BamReader reader;
    if ( !reader.Open(bamFileToOpen) ) {
	cerr << "Could not open input BAM file:" << bamFileToOpen <<endl;
    	exit(1);
    }

    reader.LocateIndex();

    if(!reader.HasIndex()){
    	cerr << "The BAM file: " << bamFileToOpen <<" does not have an index"<<endl;
    	exit(1);
    }

    // retrieve reference data
    const RefVector  references = reader.GetReferenceData();


    reader.Close();

    ////////////////////////////////////
    //   END Parsing arguments        //
    ////////////////////////////////////




       

    ////////////////////////////////////////////////////////////////////////
    //
    // BEGIN DEAMINATION PROFILE
    //
    ////////////////////////////////////////////////////////////////////////
    readNucSubstitionRatesFreq(deam5pfreqE,sub5p);
    readNucSubstitionRatesFreq(deam3pfreqE,sub3p);

    for(int nuc=0;nuc<12;nuc++){
        defaultSubMatch.s[ nuc ] = 0.0;	
    }
    ////////////////////////////////////////////////////////////////////////
    //
    // END  DEAMINATION PROFILE
    //
    ////////////////////////////////////////////////////////////////////////









    int    bpToExtract       = sizeChunk;
    
    pthread_t             thread[numberOfThreads];
    int                   rc=0;



    GenomicWindows     rw  (fastaIndex,false);
    //TODO add genomic ranges in queue
    vector<GenomicRange> v = rw.getGenomicWindows(bpToExtract,0);
    if( v.size() == 0 ){    cerr<<"No range found using these chr/loci settings"<<endl; return 1;}
    
    unsigned int rank=0;
    int lastRank=-1;

    for(unsigned int i=0;i<v.size();i++){
	//cout<<v[i]<<endl;
	DataChunk * currentChunk = new DataChunk();

	currentChunk->rangeGen = v[i];
	currentChunk->rank     = rank;
	lastRank               = rank;

	queueDataToprocess.push(currentChunk);
	rank++;
    }
    readDataDone=true;
    //    return 1;

    /////////////////////////////
    // BEGIN  Compute coverage //
    /////////////////////////////
    int bpToComputeCoverage = 1000000;
    int genomicRegionsToUse = bpToComputeCoverage/bpToExtract;
    if( genomicRegionsToUse > int(queueDataToprocess.size())){
	genomicRegionsToUse = int(queueDataToprocess.size());
    }



    queueDataForCoverage = randomSubQueue( queueDataToprocess,genomicRegionsToUse);


    pthread_mutex_init(&mutexQueue,   NULL);
    pthread_mutex_init(&mutexCounter, NULL);
    pthread_mutex_init(&mutexRank ,   NULL);

    for(int i=0;i<numberOfThreads;i++){
	rc = pthread_create(&thread[i], NULL, mainCoverageComputationThread, NULL);
	checkResults("pthread_create()\n", rc);
    }

    cout<<"Creating threads for coverage calculation"<<endl;


    while(queueDataForCoverage.size()!=0){
	cout<<getDateString()<<" "<<getTimeString()<<" # of slices left to process: "<<queueDataForCoverage.size()<<"/"<<queueDataToprocess.size()<<endl;
	sleep(timeThreadSleep);
    }
    
    //waiting for threads to finish
    for (int i=0; i <numberOfThreads; ++i) {	
	rc = pthread_join(thread[i], NULL);
	checkResults("pthread_join()\n", rc);
    }
    cout<<"Coverage computations are done"<<endl;
    pthread_mutex_destroy(&mutexRank);
    pthread_mutex_destroy(&mutexQueue);
    pthread_mutex_destroy(&mutexCounter);

    //    cout<<"Final" <<" "<<totalBasesSum<<"\t"<<totalSitesSum<<"\t"<<double(totalBasesSum)/double(totalSitesSum)<<endl;
    //    pthread_exit(NULL);
    
    rateForPoissonCov    = ((long double)totalBasesSum)/((long double)totalSitesSum);
    pdfRateForPoissonCov = pdfPoisson( rateForPoissonCov, rateForPoissonCov);


    cout<<"Results\tbp="<<totalBasesSum<<"\tsites="<<totalSitesSum<<"\tlambda="<<double(totalBasesSum)/double(totalSitesSum)<<endl;
    // for(int i=0;i<20;i++){
    // 	cout<<i<<"\t"<<pdfPoisson( (long double)i, rateForPoissonCov)/pdfPoisson( rateForPoissonCov, rateForPoissonCov)<<endl;
    // }


    // for(int i=0;i<100;i++){
    // 	cout<<i<<"\t"<<pdfPoisson( (long double)i, 20)/pdfPoisson( 20, 20)<<endl;
    // }

    //return 1;


    ////////////////////////////
    // END   Compute coverage //
    ////////////////////////////
    // doneReading=true;    

    ///////////////////////
    //  Compute hetero   //
    ///////////////////////
    cout<<"Creating threads for heterozygosity calculation"<<endl;
    pthread_mutex_init(&mutexQueue,   NULL);
    pthread_mutex_init(&mutexCounter, NULL);
    pthread_mutex_init(&mutexRank ,   NULL);

    for(int i=0;i<numberOfThreads;i++){
	rc = pthread_create(&thread[i], NULL, mainHeteroComputationThread, NULL);
	checkResults("pthread_create()\n", rc);
    }
    // 	//threads are running here

    // unsigned int originalSize = queueDataToprocess.size();
    // while(queueDataToprocess.size()!=0){
    // 	cout<<"# of slices left to process: "<<queueDataToprocess.size()<<"/"<<originalSize<<endl;
    // 	sleep(timeThreadSleep);
    // }


    ///////////////////
    //Writing data out/
    ///////////////////

    vector<GenoResults *> vectorGenoResults;
    Internal::BgzfStream bgzipWriter;

    if(outFileSiteLLFlag){
	bgzipWriter.Open(outFileSiteLL, IBamIODevice::WriteOnly);
	if(!bgzipWriter.IsOpen()){
	    cerr<<"Cannot open file "<<outFileSiteLL<<" in bgzip writer"<<endl;
	    return 1;
	}
    

	string headerOutFile;
	if(useVCFoutput){
	    headerOutFile="#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\t"+sampleName+"\n";	
	}else{
	    headerOutFile="#CHROM\tPOS\tA1\tA2\tA1c\tA2c\tGENO\tA1A1\tA1A2\tA2A2\tQualL\tCovL\n";	
	}

	bgzipWriter.Write(headerOutFile.c_str(),headerOutFile.size());
    }

    bool wroteEverything=false;
    int lastWrittenChunk=-1;   
           
    while(!wroteEverything){

	//threads are running here
	rc = pthread_mutex_lock(&mutexCounter);
	checkResults("pthread_mutex_lock()\n", rc);
	
	bool wroteData=false;
	if(!queueDataTowrite.empty()){
	
	    DataToWrite *  dataToWrite= queueDataTowrite.top();

	    if( lastWrittenChunk == (dataToWrite->rank-1) ){ 	    //correct order
		queueDataTowrite.pop();
		rc = pthread_mutex_unlock(&mutexCounter);
		checkResults("pthread_mutex_unlock()\n", rc);

		cout<<getDateString()<<" "<<getTimeString()<<" writing chunk#"<<dataToWrite->rank<<" with "<<dataToWrite->vecPositionResults->size()<<" records"<<endl;
		    
		    
		string strToWrite="";
		for(unsigned int i=0;i<dataToWrite->vecPositionResults->size();i++){
		    strToWrite += dataToWrite->vecPositionResults->at(i)->toString(references);
		    //cout<<strToWrite<<endl;
		    if( (i%500) == 499){
			if(outFileSiteLLFlag){
			bgzipWriter.Write(strToWrite.c_str(), strToWrite.size());
			}
			strToWrite="";
		    }
			
		    GenoResults * toadd =  new GenoResults( dataToWrite->vecPositionResults->at(i) );
		    vectorGenoResults.push_back(toadd);
		}

		if(!strToWrite.empty()){
		    if(outFileSiteLLFlag){
			if(outFileSiteLLFlag){ bgzipWriter.Write(strToWrite.c_str(), strToWrite.size()); }
		    }
		}
		    
		
		wroteData=true;		
		lastWrittenChunk=dataToWrite->rank;
		
		if(dataToWrite->rank == lastRank)
		    wroteEverything=true;	
		delete dataToWrite;
	    }else{
		//do nothing, we have to wait for the chunk with the right rank
		rc = pthread_mutex_unlock(&mutexCounter);
		checkResults("pthread_mutex_unlock()\n", rc);
	
	    }

	}else{//end if queue not empty
	    rc = pthread_mutex_unlock(&mutexCounter);
	    checkResults("pthread_mutex_unlock()\n", rc);
	}

	if(!wroteData)
	    sleep(timeSleepWrite);
    }

    ///////////////////////
    //end Writing data out/
    ///////////////////////



    // 	rc = pthread_mutex_lock(&mutexCounter);
    // 	checkResults("pthread_mutex_lock()\n", rc);
    // 	bool wroteData=false;
    // 	if(!queueDataTowrite.empty()){
	
    // 	    DataChunk *  dataToWrite= queueDataTowrite.top();

    // 	    if( lastWrittenChunk == (dataToWrite->rank-1) ){
    // 		queueDataTowrite.pop();
    // 		cout<<"writing "<<dataToWrite->rank<<endl;
    // 		//writing dataToWrite
    // 		for(unsigned int i=0;i<dataToWrite->dataToProcess.size();i++){
    // 		    outfile<< dataToWrite->dataToProcess[i].ids 
    // 			   << endl  
    // 			   << dataToWrite->dataToProcess[i].seqs << endl
    // 			   << "+" <<endl 
    // 			   << dataToWrite->dataToProcess[i].qual << endl;
    // 		}
		
    // 		wroteData=true;		
    // 		lastWrittenChunk=dataToWrite->rank;
		
    // 		if(dataToWrite->rank == lastRank)
    // 		    wroteEverything=true;	
    // 		delete dataToWrite;
    // 	    }else{
		
    // 	    }

    // 	}

    // 	rc = pthread_mutex_unlock(&mutexCounter);
    // 	checkResults("pthread_mutex_unlock()\n", rc);

    // 	if(!wroteData)
    // 	    sleep(timeThreadSleep);
    // }

    //waiting for threads to finish    
    for (int i=0; i <numberOfThreads; ++i) {
	rc = pthread_join(thread[i], NULL);
	checkResults("pthread_join()\n", rc);
    }
    cout<<"Heterozygosity computations are done"<<endl;
    pthread_mutex_destroy(&mutexRank);
    pthread_mutex_destroy(&mutexQueue);
    pthread_mutex_destroy(&mutexCounter);


    if(outFileSiteLLFlag){
	bgzipWriter.Close();
    }

    for(unsigned int i=0;i<vectorGenoResults.size();i++){
	delete( vectorGenoResults[i] );
    }

    pthread_exit(NULL);

    //Compute hetero rate
    


    long double h      = 0.01;
    long double lambda = 0.0000000001;

    while(true){

	// if(h>=1){
	//     h=1-espilon;
	// }
	// if(h<=0){
	//     h=espilon;
	// }
	long double probNull=0.1;	
	long double ll   = 0.0;
	long double llP  = 0.0;
	long double llPP = 0.0;

	for(int i=0;i<size;i++){
	    //log( (1-h)(1-o) + ho)
	    //log( 1-o-h-ho    +ho)
	    //f(h) = log( 1-o-h)
	    //f'(h) = 1/(1-o-h) * -1
	    //f'(h) = -1/(1-o-h) 
	    //f''(h) = (-(1-o-h)^-1)'
	    //f''(h) = -(1-o-h)^-2


	    long double probNull=0.5;
    	    //long double llT  = logl( (1-h)*(1-observations[i]) + (h)*(observations[i]) )  ;
	    long double llT  = logl( (1-errorProbabs[i])
				     *
				     ( (1-h)*(1-observations[i]) + h*observations[i] )
				     +
				     errorProbabs[i]
				     *
				     probNull);
    	    //long double llTP = (2.0*observations[i]-1)/( observations[i]*(2*h-1)-h+1 );


    	    long double llTP = 
		(  (1-errorProbabs[i])*(2.0*observations[i]-1) )
		/
		( (1-errorProbabs[i])
		  *
		  ( (1-h)*(1-observations[i]) + h*observations[i] )
		  +
		  errorProbabs[i]
		  *
		  probNull);


    	    long double llTPP = -1.0*
		(  powl((1-errorProbabs[i]),2.0) * powl((2.0*observations[i]-1),2.0) )
		/
		powl( ( (1-errorProbabs[i])
			*
			( (1-h)*(1-observations[i]) + h*observations[i] )
			+
			errorProbabs[i]
			*
			probNull),2.0);

	    

	    // long double llT  = logl( (1-errorProbabs[i])
	    // 			 *
	    // 			 ( (1-h)*(1-observations[i]) + (h)*(observations[i]) )
	    // 			 +
	    // 			 errorProbabs[i]
	    // 			 *
	    // 			 probNull  );

	    // // long double llT  = logl( (1-errorProbabs[i])
	    // // 			 *
	    // // 			 (  1-observations[i]  -h+ 2.0*h*observations[i])    
	    // // 			 +
	    // // 			 (
	    // // 			 (errorProbabs[i])
	    // // 			 *
	    // // 			 probNull ));


	    // //long double llTP = (2.0*observations[i]-1)/( observations[i]*(2*h-1)-h+1 );


	    // long double llTP = 
	    //     ( (errorProbabs[i]-1.0)*(2.0*observations[i]+1.0))
	    //       /
	    //       ( -1.0*errorProbabs[i]*(probNull-2.0*observations[i]*h+observations[i]+h-1.0 ) -2.0*observations[i]*h+observations[i]+h-1.0);

	    //       //+ observations[i]*(errorProbabs[i]*(h-1)-2*h+1 )+h-1);

	    // // long double llTP = 
	    // //     ( observations[i]*(errorProbabs[i]-2)-errorProbabs[i]+1)/
	    // //     ( errorProbabs[i]*(probNull-h) + observations[i]*(errorProbabs[i]*(h-1)-2*h+1 )+h-1);

	    // // long double llT   = logl( (1-h)*(1-observations[i]) + (h)*(observations[i]) )  ;
	    // // long double llTP  = -1.0/(1.0-observations[i]-h);
	    // long double llTPP = -1.0/powl((1.0-observations[i]-h),2.0);
	    
	    ll   += llT;
	    llP  += llTP;
	    llPP += llTPP;
	}
	
	long double errorInter= 1.96/sqrtl(-1.0*llPP);
	cout<<fixed<<h<<"\t"<<(1-h)<<"\t"<<ll<<"\t"<<llP<<"\t"<<llPP<<"\t"<<errorInter<<"\t"<<(h-errorInter)<<"\t"<<(h+errorInter)<<endl;	

	//cout<<h<<"\t"<<(1-h)<<"\t"<<ll<<"\t"<<llP<<"\t"<<llPP<<endl;	

	h=h+lambda*llP;
    }




    
    return 0;
}

