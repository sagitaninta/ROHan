/*
 * Hmm
 * Date: Dec-05-2017 
 * Author : Gabriel Renaud gabriel.reno [at sign here] gmail.com
 *
 */

#ifndef Hmm_h
#define Hmm_h

#include <iostream>
#include <vector>
#include "HmmState.h"

#include "utils.h"

#define NBRSTATES 2

using namespace std;

typedef struct { 
    double p;
    int idx;
} emission;


typedef struct { 
    vector<int> seq;
    double llik;
 } hmmpath;

class Hmm{
private:
    double **trans;

public:

    long double startingState[NBRSTATES];
    long double probTrans[NBRSTATES];
    long double probStay[NBRSTATES];
    HmmState * hmmstates[NBRSTATES];
	
    Hmm();
    Hmm(const Hmm & other);
    ~Hmm();
    Hmm & operator= (const Hmm & other);

    vector<emission> generateStates(unsigned int N,unsigned int total) const;
    int getNumberStates() const;
    double getTrans(int i,int j) const;
    void setHetRateForNonROH(long double newH);
    void setTransprob(long double newTrans);
    
    friend ostream& operator<<(ostream& os, const Hmm& hmm);  

};


inline double logRobust(double p_){
    double p = p_;
    if(p == 0){
	p=numeric_limits<double>::epsilon();
    }

    return log(p);
}





inline double forwardProb (Hmm * hmm, const vector<double> & observed, unsigned int sizeChunk){
    int nObservations  = int(observed.size());
    int nStates        = hmm->getNumberStates(); 

    vector< vector<double > > f ( nStates , vector<double>(nObservations,0) );//1D # HMM states, 2D #obs,  probability of observation i by state j

    for (int state = 0; state < nStates; state++) { //
	f[state][0] =
	    logRobust( hmm->startingState[state]) +                                                                //prob of starting a state
	    logRobust( hmm->hmmstates[state]->probEmission( (unsigned int)(observed[0]*sizeChunk)  , sizeChunk) ); //emitting observed[0] by state
    }


    for (int k = 1; k < nObservations; k++) {//each obs

	for (int state = 0; state < nStates; state++) {//each state
	    double logsum = -1.0*numeric_limits<double>::infinity();
	    // double p;
	    // int dmax=-1;

	    for (int previousState = 0; previousState < nStates; previousState++) {//each previous state
		double temp =
		    f[previousState][k-1] +                         // prob stored in previousState
		    logRobust(hmm->getTrans(previousState,state));  // prob of transition from previousState to state
		logsum = oplusInitnatl( logsum, temp );
	    }//end each previous state

	    //logsum contains the sum of all probs from every previous state
	    
	    f[state][k] =
		logRobust( hmm->hmmstates[state]->probEmission( (unsigned int)(observed[k]*sizeChunk)  , sizeChunk) ) +  //emission probability by state
		logsum;                                                                                                  //sum of all probs for every previous state	    

	}//each state

    }//end each obs
    

    double temp=0;
    for (int state = 0; state < nStates; state++) {//each state
	temp = oplusInitnatl( temp ,  	    f[state][nObservations-1]  );
    }

    return temp;
}


inline vector< vector<double> > forward (Hmm * hmm, const vector<double> & observed, unsigned int sizeChunk){
    // hmm$transProbs[is.na(hmm$transProbs)]       = 0
    // hmm$emissionProbs[is.na(hmm$emissionProbs)] = 0
    // nObservations  = length(observation)
    // 	nStates        = length(hmm$States)
    int nObservations  = int(observed.size());
    int nStates        = hmm->getNumberStates(); 
    //f          = array(NA,c(nStates,nObservations))

    vector< vector<double > > f ( nStates , vector<double>(nObservations,0) );//1D # HMM states, 2D #obs,  probability of observation i by state j

    // for (int j = 0; j < hmm->getNumberStates(); j++) {//each state
	
    // 	vector<double> toaddProb;
	
    // 	for (int i = 0; i < nObservations; i++) {//each obs
	    
    // 	    toaddProb.push_back(0.0);
    // 	}
	
    // 	f.push_back(  toaddProb );
    // }


    // Init
    // for(state in hmm$States)  {
    // 	f[state,1] = log(hmm$startProbs[state] * hmm$emissionProbs[state,observation[1]])
    // 	    }
    for (int state = 0; state < nStates; state++) { //
	f[state][0] =
	    logRobust( hmm->startingState[state]) +                                                                //prob of starting a state
	    logRobust( hmm->hmmstates[state]->probEmission( (unsigned int)(observed[0]*sizeChunk)  , sizeChunk) ); //emitting observed[0] by state
    }


    for (int k = 1; k < nObservations; k++) {//each obs
	cout<<observed[k]<<" ";
	for (int state = 0; state < nStates; state++) {//each state
	    double logsum = -1.0*numeric_limits<double>::infinity();
	    // double p;
	    // int dmax=-1;

	    for (int previousState = 0; previousState < nStates; previousState++) {//each previous state
		double temp =
		    f[previousState][k-1] +                         // prob stored in previousState
		    logRobust(hmm->getTrans(previousState,state));  // prob of transition from previousState to state
		//if(temp > (-1.0*numeric_limits<double>::infinity()) ) {
		//logsum = temp + log(1 + exp(logsum - temp ));
		logsum = oplusInitnatl( logsum, temp );
		// }else{
		//     cout<<"temp skipped"<<endl;
		// }		
		// cout<<"obs#"<<k<<" "<<observed[k]<< " state#"<<state<<" prevState#"<<previousState<<" temp="<<temp<<" prev="<< f[previousState][k-1] <<" log="<<logRobust(hmm->getTrans(previousState,state))<<" logsum="<<logsum<<endl;
	    }//end each previous state

	    //logsum contains the sum of all probs from every previous state
	    
	    f[state][k] =
		logRobust( hmm->hmmstates[state]->probEmission( (unsigned int)(observed[k]*sizeChunk)  , sizeChunk) ) +  //emission probability by state
		logsum;                                                                                                  //sum of all probs for every previous state
	    
	    cout<<"f["<<state<<"]["<<k<<"] "<<f[state][k]<<" ";


	}//each state
	cout<<endl;
    }//end each obs
    // cerr<<"done forward()"<<endl;

    //exit(1);
    
    // # Iteration
    //   for(k in 2:nObservations)
    //   {
    //     for(state in hmm$States)
    //     {
    //       logsum = -Inf
    //       for(previousState in hmm$States)
    //       {
    //         temp   = f[previousState,k-1] + log(hmm$transProbs[previousState,state])
    // 		if(temp > - Inf)
    // 		{
    // 			logsum = temp + log(1 + exp(logsum - temp ))
    // 		}
    //       }
    //       f[state,k] = log(hmm$emissionProbs[state,observation[k]]) + logsum
    //     }
    //   }
  
    return f;
}





inline vector< vector<double> > backward (Hmm * hmm, const vector<double> & observed, unsigned int sizeChunk){
    // backward = function(hmm, observation)
    // {
    //   hmm$transProbs[is.na(hmm$transProbs)]       = 0
    //   hmm$emissionProbs[is.na(hmm$emissionProbs)] = 0
    //   nObservations  = length(observation)
    //   nStates    = length(hmm$States)
    //   b          = array(NA,c(nStates,nObservations))
    //   dimnames(b)= list(states=hmm$States,index=1:nObservations)

    int nObservations  = int(observed.size());
    int nStates        = hmm->getNumberStates(); 

    vector< vector<double > > b ( nStates , vector<double>(nObservations,0) );//1D # HMM states, 2D #obs,  probability of observation i by state j

    //   # Init
    //   for(state in hmm$States)
    //   {
    //     b[state,nObservations] = log(1)
    //   }

    // for (int j = 0; j < nStates; j++) {//each state	
    // 	vector<double> toaddProb;	
    // 	for (int i = 0; i < nObservations; i++) {//each obs	    
    // 	    toaddProb.push_back(0.0);
    // 	}	
    // 	b.push_back(  toaddProb );
    // }
    
    for (int k=(nObservations-2); k>=0; k--) {//each obs
	cout<<k<<" "<<observed[k]<<" "<<observed[k+1]<<" ";
	for (int state=0; state<nStates; state++) {//each state
	    double logsum = -1.0*numeric_limits<double>::infinity();

	    for (int nextState = 0; nextState < nStates; nextState++) {//each next state
		double temp =
		    b[nextState][k+1] + 
		    logRobust(hmm->getTrans(state,nextState) ) +                                                               //transition from state to nextState
		    logRobust(hmm->hmmstates[nextState]->probEmission( (unsigned int)(observed[k+1]*sizeChunk)  , sizeChunk)); //emission probability of state 
		//cout<<"b["<<nextState<<"]["<<(k+1)<<"] "<<f[state][k]<<" = logRobust = "<<logRobust(hmm->hmmstates[state]->probEmission( (unsigned int)(observed[k]*sizeChunk)  , sizeChunk) ) << " logsum = "<<logsum<<endl;
		
		logsum = oplusInitnatl( logsum, temp );
		cout<<" s="<<state<<" n="<<nextState<<" t="<<temp<<" "<<
		    "b="<<b[nextState][k+1]<<" "
		    "tr="<<hmm->getTrans(state,nextState) <<" pe="<<logRobust(hmm->hmmstates[nextState]->probEmission( (unsigned int)(observed[k+1]*sizeChunk)  , sizeChunk))<<" ";
	    }//end each next state

	    cout<<"b["<<state<<"]["<<k<<"] = "<<logsum<<" ";
	    b[state][k] = logsum;

	}//each state
	cout<<endl;
    }//end each obs
    
    return b;
    
//   # Iteration
//   for(k in (nObservations-1):1)
//   {
//     for(state in hmm$States)
//     {
//       logsum = -Inf
//       for(nextState in hmm$States)
//       {
//         temp   = b[nextState,k+1] + log(hmm$transProbs[state,nextState]*hmm$emissionProbs[nextState,observation[k+1]])
// 		if(temp > - Inf)
// 		{
//         	logsum = temp + log(1 + exp(logsum-temp))
// 		}
//       }
//       b[state,k] = logsum
//     }
//   }
//    return(b)
}



inline double forwardBackward(Hmm * hmm, const vector<double> & observed, unsigned int sizeChunk){

    /* cout<<"forwardBackward"<<endl; */
    /* exit(1); */
    // baumWelchRecursion = function(hmm, observation){
    int nObservations  = int(observed.size());
    int nStates        = hmm->getNumberStates(); 

    vector< vector<double> >  f = forward( hmm,  observed, sizeChunk);
    vector< vector<double> >  b = backward(hmm,  observed, sizeChunk);

    // double probObservations = f[0][nObservations-1];
    // for(int i=1;i<nStates;i++){
    // 	double j = f[i,nObservations-1];
    // 	probObservations = oplusInitnatl( j, probObservations );
    // }


    ///    exit(1);
    vector< vector<double> > postProb (nStates,vector<double>(nObservations));

    /* cout<<"FWBW test postprob"<<endl; */
    for(int i=0;i<nObservations;i++){
	double temp = 0.0;
	cout<<"obs#"<<i<<" "<<observed[i];
	for(int x=0;x<nStates;x++){	    
	    postProb[x][i]   = f[x][i] + b[x][i];
	    /* cout<<" state#"<<x<<" p="<<postProb[x][i]<<" f="<<f[x][i] <<" b="<< b[x][i]<<" "; */
	    cout<<" state#"<<x<<" p="<<postProb[x][i]<<" "<<temp<<" "; 

	    temp             = oplusInitnatl(temp,postProb[x][i]);
	    cout<<" "<<temp<<" ";
	}
	cout<<" sum"<<temp<<endl;
	
	for(int x=0;x<nStates;x++){
	    postProb[x][i]  -= temp;
	    cout<<"Norm. postProb#"<<x<<" p="<<postProb[x][i]<<" ";
	}
	cout<<endl;
    }

    /* cout<<"posterior prob"<<endl; */
    double sumProb=0.0;
    for(int i=0;i<nObservations;i++){
	cout<<"obs#"<<i<<" "<<observed[i];
	double temp = 0.0;
	for(int x=0;x<nStates;x++){
	    cout<<" "<<x<<" "<<postProb[x][i]; 
	    temp             = oplusInitnatl(temp,postProb[x][i]);
	}
	cout<<" "<<temp<<endl;
	sumProb+=temp;

    }
    return sumProb;
    /* exit(1); */

}

// for(int x=0;x<nStates;x++){
// 	for(int y=0;y<nStates;y++){
// 	    //            forward up to x+trans x to y                    + emission observation of observed[0+1] from y                                                      + backward from y onwards
// 	    double temp = f[x][0]        + logRobust(hmm->getTrans(x,y) ) + logRobust(hmm->hmmstates[y]->probEmission( (unsigned int)(observed[0+1]*sizeChunk)  , sizeChunk)) + b[y][0+1];
// 	    
// 		// forward up to x+ trans x to y                   + emission observation of observed[i+1] from y                                                      + backward from y onwards
// 		j =       f[x][i] + logRobust(hmm->getTrans(x,y) ) + logRobust(hmm->hmmstates[y]->probEmission( (unsigned int)(observed[i+1]*sizeChunk)  , sizeChunk)) + b[y][i+1];
// 		temp = oplusInitnatl( j, temp );

// 		//TODO set transition matrix
// 	    }
// 	}
// }


/*
void baumWelchRecursion (Hmm * hmm, const vector<double> & observed){
    // baumWelchRecursion = function(hmm, observation){
    int nObservations  = int(observed.size());
    int nStates        = hmm->getNumberStates(); 

    // 	TransitionMatrix    = hmm$transProbs
    // 	TransitionMatrix[,] = 0
    // 	EmissionMatrix      = hmm$emissionProbs
    // 	EmissionMatrix[,]   = 0
    // 	f = forward(hmm,  observation)
    // 	b = backward(hmm, observation)
    vector< vector<double> >  f = forward( hmm,  observed);
    vector< vector<double> >  b = backward(hmm,  observed);

    double probObservations = f[0][nObservations-1];
    for(int i=1;i<nStates;i++){
	double j = f[i,nObservations-1];
	probObservations = oplusInitnatl( j, probObservations );
    }
// 		if(j > - Inf)
// 		{
// 			probObservations = j + log(1+exp(probObservations-j))
// 		}

//}
// 	probObservations = f[1,length(observation)]
// 	for(i in 2:length(hmm$States))
// 	{
// 		j = f[i,length(observation)]
// 		if(j > - Inf)
// 		{
// 			probObservations = j + log(1+exp(probObservations-j))
// 		}
// 	}


    for(int x=0;x<nStates;x++){
	for(int y=0;y<nStates;y++){
	    //            forward up to x+trans x to y                    + emission observation of observed[0+1] from y                                                      + backward from y onwards
	    double temp = f[x][0]        + logRobust(hmm->getTrans(x,y) ) + logRobust(hmm->hmmstates[y]->probEmission( (unsigned int)(observed[0+1]*sizeChunk)  , sizeChunk)) + b[y][0+1];
	    for(int i=1;i<(length(observation)-1);i++){
		// forward up to x+ trans x to y                   + emission observation of observed[i+1] from y                                                      + backward from y onwards
		j =       f[x][i] + logRobust(hmm->getTrans(x,y) ) + logRobust(hmm->hmmstates[y]->probEmission( (unsigned int)(observed[i+1]*sizeChunk)  , sizeChunk)) + b[y][i+1];
		temp = oplusInitnatl( j, temp );

		//TODO set transition matrix
	    }
	}
    }

    
// 	for(x in hmm$States)
// 	{
// 		for(y in hmm$States)
// 		{
// 			temp = f[x,1] + log(hmm$transProbs[x,y]) +
// 					log(hmm$emissionProbs[y,observation[1+1]]) + b[y,1+1]
// 			for(i in 2:(length(observation)-1))
// 			{
// 				j = f[x,i] + log(hmm$transProbs[x,y]) +
// 						log(hmm$emissionProbs[y,observation[i+1]]) + b[y,i+1]
// 				if(j > - Inf)
// 				{
// 					temp = j + log(1+exp(temp-j))
// 				}
// 			}
// 			temp = exp(temp - probObservations)
// 			TransitionMatrix[x,y] = temp
// 		}
// 	}

    for(int x=0;x<nStates;x++){

	
// 	for(x in hmm$States)
// 	{
// 		for(s in hmm$Symbols)
// 		{
// 			temp = -Inf
// 			for(i in 1:length(observation))
// 			{
// 				if(s == observation[i])
// 				{
// 					j = f[x,i] + b[x,i]
// 					if(j > - Inf)
// 					{
// 						temp = j + log(1+exp(temp-j))
// 					}
// 				}
// 			}
// 			temp = exp(temp - probObservations)
// 			EmissionMatrix[x,s] = temp
// 		}
// 	}
// 	//return(list(TransitionMatrix=TransitionMatrix,EmissionMatrix=EmissionMatrix))
// }

//taken from https://cran.r-project.org/web/packages/HMM/
void baum_welch(Hmm * hmm,		          // HMM to optimize
 		const vector<double> * emittedH   // observations
){ 
    Hmm tempHmm (*hmm);
    int maxIterations=10;
    for(int i=0;i<maxIterations;i++){

	//Expectation Step (Calculate expected Transitions and Emissions)
	//bw = baumWelchRecursion(tempHmm, observation);
	// T  = bw$TransitionMatrix;
	// E  = bw$EmissionMatrix;
	// // Pseudocounts
	// T[!is.na(hmm$transProbs)]    = T[!is.na(hmm$transProbs)]    + pseudoCount;
	// E[!is.na(hmm$emissionProbs)] = E[!is.na(hmm$emissionProbs)] + pseudoCount;
	// // Maximization Step (Maximise Log-Likelihood for Transitions and Emissions-Probabilities)
	// T = (T/apply(T,1,sum));
	// E = (E/apply(E,1,sum));
	// d = sqrt(sum((tempHmm$transProbs-T)^2)) + sqrt(sum((tempHmm$emissionProbs-E)^2));
	// diff = c(diff, d);
	// tempHmm$transProbs    = T;
	// tempHmm$emissionProbs = E;
	// if(d < delta){
	//     break;
	// }

    }
    // tempHmm$transProbs[is.na(hmm$transProbs)]       = 0
	// tempHmm$emissionProbs[is.na(hmm$emissionProbs)] = 0
	// diff = c()
	// for(i in 1:maxIterations)
	// {
	// 	# Expectation Step (Calculate expected Transitions and Emissions)
	// 	bw = baumWelchRecursion(tempHmm, observation)
	// 	T  = bw$TransitionMatrix
	// 	E  = bw$EmissionMatrix
	// 	# Pseudocounts
	// 	T[!is.na(hmm$transProbs)]    = T[!is.na(hmm$transProbs)]    + pseudoCount
	// 	E[!is.na(hmm$emissionProbs)] = E[!is.na(hmm$emissionProbs)] + pseudoCount
	// 	# Maximization Step (Maximise Log-Likelihood for Transitions and Emissions-Probabilities)
	// 	T = (T/apply(T,1,sum))
	// 	E = (E/apply(E,1,sum))
	// 	d = sqrt(sum((tempHmm$transProbs-T)^2)) + sqrt(sum((tempHmm$emissionProbs-E)^2))
	// 	diff = c(diff, d)
	// 	tempHmm$transProbs    = T
	// 	tempHmm$emissionProbs = E
	// 	if(d < delta)
	// 	{
	// 		break
	// 	}
	// }
	// tempHmm$transProbs[is.na(hmm$transProbs)]       = NA
	// tempHmm$emissionProbs[is.na(hmm$emissionProbs)] = NA
	// return(list(hmm=tempHmm,difference=diff))
}
*/

// void baum_welch(Hmm * hmm,		          // HMM to optimize
// 		const vector<double> * emittedH   // observations
// ){ 
//     const int length    = emittedH->size();
//     //const int no_chunks = B.get_no_chunks_per_row();

    
//     // sequence &obsseq,
//     // const HMMMatrix<float_type, sse_float_type> &F,
//     // const HMMMatrix<float_type, sse_float_type> &B, 
//     // const HMMVector<float_type, sse_float_type> &scales,
//     // HMMVector<float_type, sse_float_type> &new_pi,
//     // HMMMatrix<float_type, sse_float_type> &new_T,
//     // HMMMatrix<float_type, sse_float_type> &new_E) {
    

// }

/*
//https://github.com/tttamaki/HMMLib/blob/master/HMMlib/hmm.hpp 
void baum_welch(const sequence &obsseq,
		const HMMMatrix<float_type, sse_float_type> &F,
		const HMMMatrix<float_type, sse_float_type> &B, 
		const HMMVector<float_type, sse_float_type> &scales,
		HMMVector<float_type, sse_float_type> &new_pi,
		HMMMatrix<float_type, sse_float_type> &new_T,
		HMMMatrix<float_type, sse_float_type> &new_E) {
		
    // easier reference
    HMMMatrix<float_type, sse_float_type> &T = *trans_prob;    //transition prob
    HMMMatrix<float_type, sse_float_type> &E = *emission_prob; //emission   prob
		
    const int length = obsseq.size();
    const int no_chunks = B.get_no_chunks_per_row();
		
    // sanity check
    assert(new_T.get_no_columns() == no_states);
    assert(new_T.get_no_rows() ==  no_states);
    assert(new_E.get_no_columns() == no_states);
    assert(new_E.get_no_rows() == alphabet_size);
    assert(new_pi.get_size() == no_states);
    assert(scales.get_size() == length);
    assert(F.get_no_rows() == length);
    assert(F.get_no_columns() == no_states);
    assert(B.get_no_rows() == length);
    assert(B.get_no_columns() == no_states);
		
    // initialise
    new_pi.reset();
    new_T.reset();
    new_E.reset();

    // compute counts
    HMMVector<float_type, sse_float_type> pi_counts(no_states);
    HMMMatrix<float_type, sse_float_type> T_counts(no_states, no_states);
    HMMMatrix<float_type, sse_float_type> E_counts(alphabet_size, no_states);
		
    // compute new_pi and initialize E_counts
    unsigned x = obsseq[0];
    sse_float_type scale;
    sse_operations_traits::set_all(scale, (float_type) (1.0/scales(0)));
    for (int sc = 0; sc < no_chunks; ++sc) {
	sse_float_type temp = (F.get_chunk(0,sc) * B.get_chunk(0,sc)) * scale;
	new_pi.get_chunk(sc) = temp;
	E_counts.get_chunk(x, sc) += temp;
    }
		
    // compute transition and emission counts
    for (int i = 1; i < length; ++i) {
	x = obsseq[i];
#ifdef WITH_OMP
#pragma omp parallel for
#endif
	for(int j = 0; j < no_states; ++j) {
	    // transition counts
	    sse_float_type prev_forward;
	    sse_operations_traits::set_all(prev_forward, F(i-1, j));
	    for(int chunk = 0; chunk < no_chunks; ++chunk) {
		T_counts.get_chunk(j, chunk) += prev_forward * 
		    T.get_chunk(j,chunk) * 
		    E.get_chunk(x,chunk) * 
		    B.get_chunk(i,chunk);
	    }
	}
	// emission counts
	sse_operations_traits::set_all(scale, (float_type) (1.0/scales(i)));
	for(int chunk = 0; chunk < no_chunks; ++chunk)
	    E_counts.get_chunk(x,chunk) += F.get_chunk(i, chunk) * 
		B.get_chunk(i, chunk) * scale;
    }

    // compute new_T and new_E by normalizing counts
    for (int s = 0; s < no_states; ++s) {
	// transition probabilities
	float_type sum = 0.0;
	for (int dst_state = 0; dst_state < no_states; ++dst_state)
	    sum += T_counts(s, dst_state);
	for (int dst_state = 0; dst_state < no_states; ++dst_state)
	    new_T(s, dst_state) = T_counts(s, dst_state) / sum;

	// emission probabilities
	sum = 0.0;
	for (int sym = 0; sym < alphabet_size; ++sym)
	    sum += E_counts(sym, s);
	for (int sym = 0; sym < alphabet_size; ++sym)
	    new_E(sym, s) = E_counts(sym, s) / sum;
    }
}
*/

//taken from https://gist.github.com/Feder1co5oave/2347228

inline hmmpath viterbi(Hmm * hmm, const vector<double> & observed, unsigned int sizeChunk){//, const int n) {
    int n = int(observed.size());

    //assert(n > 0); assert(observed != NULL);
    //int *seq = new int[n];
    //vector<int> seq =
    hmmpath toreturn;
    toreturn.seq = vector<int>(n,0);
    
    //add log like
    for (int i = 0; i < n; i++)
	toreturn.seq[i] = 0;
	//	seq[i] = 0;
    // if(hmm->getNumberStates() > n){
    // 	cerr<<"Error in Viterbi(), there are more HMM states than 
    // }
    
    //vector< vector<double> > prob;  //1D #obs, 2D # HMM states, probability of observation i by state j
    vector< vector<double> > prob ( hmm->getNumberStates() , vector<double>(n,0.0) );//1D # HMM states, 2D #obs,  probability of observation i by state j
    //vector< vector<int>    > prevs; //1D #obs, 2D # HMM states, most likely state for observation i at state j
    vector< vector<int>    > prevs ( hmm->getNumberStates() ,   vector<int>(n,  0) );//1D # HMM states, 2D #obs,  most likely state for observation i at state j
    
    // for (int i = 0; i < n; i++) {//each obs
    // 	vector<double> toaddProb;
    // 	vector<int>    toaddPrev;
	
    // 	for (int j = 0; j < hmm->getNumberStates(); j++) {//each state
    // 	    toaddProb.push_back(0.0);
    // 	    toaddPrev.push_back(  0);	    
    // 	}

    // 	prob.push_back(  toaddProb );
    // 	prevs.push_back( toaddPrev );
	
    // }
    
    // double **prob = new double*[n]; //probability of the observation given 
    // int **prevs   = new int*[n];

    //
    // for (int i = 0; i < n; i++) {
    // 	prob[i]  = new double[hmm->getNumberStates()];
    // 	prevs[i] = new int[hmm->getNumberStates()];
    // 	for (int j = 0; j < hmm->getNumberStates(); j++) { //unnecessary
    // 	    prob[i][j] = 0;
    // 	    prevs[i][j] = 0;
    // 	}
    // }


    //Initializing the matrix using starting probabilities for observation 0 
    for (int i = 0; i < hmm->getNumberStates(); i++) { //
	//prob[0][i] = hmm->startingState[i] * hmm->hmmstates[i]->probEmission( (unsigned int)(observed[0]*sizeChunk)  , sizeChunk);
	// double pss = hmm->startingState[i];
	// if(pss==0){
	//     pss=numeric_limits<double>::epsilon();
	// }
	// double pe = hmm->hmmstates[i]->probEmission( (unsigned int)(observed[0]*sizeChunk)  , sizeChunk);
	// if(pe==0){
	//     pe=numeric_limits<double>::epsilon();
	// }
	
	//prob[0][i] = log( pss ) + log(pe);
	prob[0][i] = logRobust(hmm->startingState[i]) + logRobust( hmm->hmmstates[i]->probEmission( (unsigned int)(observed[0]*sizeChunk)  , sizeChunk) );
	cout<<"prob0 "<<observed[0]<<" "<<i<<" "<<prob[0][i]<<" "<<endl;
	//hmm->B[i][ observed[0] ];
	//prob[0][i] = hmm->PI[i] * hmm->B[i][ observed[0] ]; //state i emit char 0
    }
    
    //exit(1);


    //Populate the prob[i][j] for observation i at state j using the previous computations at the previous state and previous observation
    for (int i = 1; i < n; i++) {//each obs

	for (int j = 0; j < hmm->getNumberStates(); j++) {//each state
	    double pmax = -1.0*numeric_limits<double>::infinity();
	    double p;
	    int dmax=-1;

	    for (int k = 0; k < hmm->getNumberStates(); k++) {//each previous state
		//p = prob[i-1][k] * hmm->A[k][j];
		//p = prob[i-1][k] * hmm->getTrans(k,j);
		p = prob[i-1][k] + logRobust(hmm->getTrans(k,j));
		
		
		if (p > pmax) {
		    pmax = p;
		    dmax = k;
		}
		cout<<"state#"<<k<<" "<<p<<" "<<pmax<<" "<<dmax<<endl;
	    }//end each previous state
	    cout<<"-------"<<endl;
	    if(dmax==-1){
		cerr<<"Error at obs "<<i<<" state#"<<j<<endl;
		exit(1);
	    }
	    
	    //prob[i][j] = hmm->B[j][ observed[i] ] * pmax;
	    //prob[i][j]    = hmm->hmmstates[j]->probEmission( (unsigned int)(observed[i]*sizeChunk)  , sizeChunk) * pmax;
	    cerr<<"["<<i<<"]["<<j<<"]"<<endl;
	    prob[i][j]    = logRobust(hmm->hmmstates[j]->probEmission( (unsigned int)(observed[i]*sizeChunk)  , sizeChunk) ) + pmax ;
	    prevs[i-1][j] = dmax;
	}//each state
    }//end each obs
    cerr<<"done"<<endl;
    
    double pmax =  -1.0*numeric_limits<double>::infinity();
    int    dmax =-1;

    //finding max at last observation
    for (int i = 0; i < hmm->getNumberStates(); i++) {
	if (prob[n-1][i] > pmax) {
	    pmax = prob[n-1][i];
	    dmax = i;
	}
    }
    
    if(dmax==-1){
	cerr<<"Error at last obs "<<n-1<<endl;
	exit(1);
    }
    //seq[n-1] = dmax;
    toreturn.seq[n-1] = dmax;
    
    toreturn.llik = pmax;

    //reconstructing the path
    
    for (int i = n-2; i >= 0; i--) {
	//seq[i] = prevs[i][ seq[i+1] ];
	toreturn.seq[i] = prevs[i][ toreturn.seq[i+1] ];
    }
    
	
    //////////////////////////////////////////////////////////
    // for (int i = 0; i < n; i++) {
    // 	cout << "t = " << i << endl;
    // 	for (int j = 0; j < hmm->getNumberStates(); j++) {
    // 	    cout << '[' << j << ']' << prob[i][j] << ' ';
    // 	}
    // 	cout << "\n\n";
    // }
	
    // for (int i = 0; i < n; i++) {
    // 	cout << "t = " << i << endl;
    // 	for (int j = 0; j < hmm->getNumberStates(); j++) {
    // 	    cout << '[' << j << ']' << prevs[i][j] << ' ';
    // 	}
    // 	cout << "\n\n";
    // }	
	
    // cout << endl;
    // for (int i = 0; i < n; i++)
    // 	cout  << i << "\t" << observed[i] <<"\t"<< toreturn.seq[i] << endl;
    // cout << endl;
    //////////////////////////////////////////////////////////
	
	
    // for (int i = 0; i < n; i++) {
    // 	delete[] prob[i];
    // 	delete[] prevs[i];
    // }
    // delete[] prob;
    // delete[] prevs;

    return toreturn;
    //return seq;
}

#endif
