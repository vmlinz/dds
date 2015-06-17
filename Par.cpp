/*
   DDS 2.6.0   A bridge double dummy solver.
   Copyright (C) 2006-2014 by Bo Haglund
   Cleanups and porting to Linux and MacOSX (C) 2006 by Alex Martelli.
   The code for calculation of par score / contracts is based upon the
   perl code written by Matthew Kidd for ACBLmerge. He has kindly given
   me permission to include a C++ adaptation in DDS.
*/

/*
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
   implied.  See the License for the specific language governing
   permissions and limitations under the License.
*/

/*#include "stdafx.h"*/
#include "dll.h"
#include "dds.h"
#include "Par.h"

int stat_contr[5]={0,0,0,0,0};
const int max_low[3][8] = {{0,0,1,0,1,2,0,0},{0,0,1,2,0,1,0,0},{0,0,1,2,3,0,0,0}};  /* index 1: 0=NT, 1=Major, 2=Minor  index 2: contract level 1-7 */


int STDCALL CalcParPBN(struct ddTableDealPBN tableDealPBN, 
  struct ddTableResults * tablep, int vulnerable, struct parResults *presp) {
  int res;
  struct ddTableDeal tableDeal;
  int ConvertFromPBN(char * dealBuff, unsigned int remainCards[4][4]);
  int STDCALL CalcPar(struct ddTableDeal tableDeal, int vulnerable, 
    struct ddTableResults * tablep, struct parResults *presp);

  if (ConvertFromPBN(tableDealPBN.cards, tableDeal.cards)!=1)
    return RETURN_PBN_FAULT;

  res=CalcPar(tableDeal, vulnerable, tablep, presp);

  return res;
}

#ifdef DEALER_PAR_ENGINE_ONLY

int STDCALL CalcPar(struct ddTableDeal tableDeal, int vulnerable, 
    struct ddTableResults * tablep, struct parResults *presp) {

  int res, i, k, m;
  struct parResultsDealer sidesRes[2];
  struct parContr2Type parContr2[10];
  
  int CalcMultiContracts(int max_lower, int tricks);

  res=CalcDDtable(tableDeal, tablep);

  if (res!=1)
    return res;

  res = SidesPar(tablep, sidesRes, vulnerable);
  if (res != 1)
    return res;
 
  for (k=0; k<16; k++) {
    presp->parScore[0][k]='\0';
    presp->parScore[1][k]='\0';
  }

  sprintf(presp->parScore[0], "NS %d", sidesRes[0].score);
  sprintf(presp->parScore[1], "EW %d", sidesRes[1].score);

  for (k=0; k<128; k++) {
    presp->parContractsString[0][k] = '\0';
    presp->parContractsString[1][k] = '\0';
  }

  strcat(presp->parContractsString[0], "NS:");
  strcat(presp->parContractsString[1], "EW:");

  char one_contr[4];
  for ( m=0; m<3; m++)
    one_contr[m]='\0';

  char strain_contr[2] = {'0','\0'};

  for (i=0; i<2; i++) { 

    if (sidesRes[i].score == 0) 
      continue;

    if (sidesRes[i].contracts[0][2] == '*') {
      /* Sacrifice */  

      for (k = 0;  k<sidesRes[i].number; k++) {

        for (int u=0; u<10; u++)
          parContr2[k].contracts[u]=sidesRes[i].contracts[k][u];          

        if (sidesRes[i].contracts[k][1]=='N')
	  parContr2[k].denom=0;
	else if (sidesRes[i].contracts[k][1]=='S')
	  parContr2[k].denom=1;
        else if (sidesRes[i].contracts[k][1]=='H')
	  parContr2[k].denom=2;
	else if (sidesRes[i].contracts[k][1]=='D')
	  parContr2[k].denom=3;
	else if (sidesRes[i].contracts[k][1]=='C')
	  parContr2[k].denom=4;
      }

      for (int s = 1; s < sidesRes[i].number; s++) {
        struct parContr2Type tmp = parContr2[s]; 
        int r = s; 
        for (; r && tmp.denom < parContr2[r - 1].denom ; --r) 
          parContr2[r] = parContr2[r - 1]; 
        parContr2[r] = tmp; 
      }


      for (int t=0; t<sidesRes[i].number; t++) {

	if (t != 0)
	  strcat(presp->parContractsString[i], ",");
  
	if (parContr2[t].contracts[5] == 'W')
	  strcat(presp->parContractsString[i], "EW ");
	else if (parContr2[t].contracts[5] == 'S')   
          strcat(presp->parContractsString[i], "NS ");
	else {
	  switch (parContr2[t].contracts[4]) {
	    case 'N': strcat(presp->parContractsString[i], "N "); break;
            case 'S': strcat(presp->parContractsString[i], "S "); break;
	    case 'E': strcat(presp->parContractsString[i], "E "); break;
	    case 'W': strcat(presp->parContractsString[i], "W "); break;
          }
        }
	for (m=0; m<2; m++)
	  one_contr[m] = parContr2[t].contracts[m];
	one_contr[2] = 'x';
	one_contr[3] = '\0';
	strcat(presp->parContractsString[i], one_contr); 
      }

    }
    else {
	/* Contract(s) make */

      char levels_coll[12];
      for (m=0; m<12; m++)
        levels_coll[m]='\0';

      for (k = 0; k<sidesRes[i].number; k++) {
	for (int u = 0; u<10; u++)
	  parContr2[k].contracts[u] = sidesRes[i].contracts[k][u];

	if (sidesRes[i].contracts[k][1] == 'N')
	  parContr2[k].denom = 0;
	else if (sidesRes[i].contracts[k][1] == 'S')
	  parContr2[k].denom = 1;
	else if (sidesRes[i].contracts[k][1] == 'H')
	  parContr2[k].denom = 2;
	else if (sidesRes[i].contracts[k][1] == 'D')
	  parContr2[k].denom = 3;
	else if (sidesRes[i].contracts[k][1] == 'C')
	  parContr2[k].denom = 4;
      }

      for (int s = 1; s < sidesRes[i].number; s++) {
	struct parContr2Type tmp = parContr2[s];
	int r = s;
	for (; r && tmp.denom < parContr2[r - 1].denom; --r)
	  parContr2[r] = parContr2[r - 1];
	parContr2[r] = tmp;
      }

      for (int t = 0; t<sidesRes[i].number; t++) {
	if (t != 0)
	  strcat(presp->parContractsString[i], ",");

        if (parContr2[t].contracts[4] == 'W')
	  strcat(presp->parContractsString[i], "EW ");
	else if (parContr2[t].contracts[4] == 'S')
          strcat(presp->parContractsString[i], "NS ");
	else {
	  switch (parContr2[t].contracts[3]) {
	    case 'N': strcat(presp->parContractsString[i], "N "); break;
            case 'S': strcat(presp->parContractsString[i], "S "); break;
	    case 'E': strcat(presp->parContractsString[i], "E "); break;
	    case 'W': strcat(presp->parContractsString[i], "W "); break;
          }
        }

	for (m=0; m<2; m++)
	  one_contr[m] = parContr2[t].contracts[m];
	one_contr[2]='\0';

	strain_contr[0]=one_contr[1];

	char * ptr_c = strchr(parContr2[t].contracts, '+');
	if (ptr_c != NULL) {
	  ptr_c++;
	  int add_contr = (*ptr_c) - 48;


          sprintf(levels_coll, "%d", 
		CalcMultiContracts(add_contr, 
		(parContr2[t].contracts[0] - 48) + 6 + add_contr));

	  strcat(presp->parContractsString[i], levels_coll);
	  strcat(presp->parContractsString[i], strain_contr);
	  
        }
	else {
	  strcat(presp->parContractsString[i], one_contr);
	
	}
      }
    }
  }

  return res;
}

#else
int STDCALL CalcPar(struct ddTableDeal tableDeal, int vulnerable, 
    struct ddTableResults * tablep, struct parResults *presp) {

  int res;

  res=CalcDDtable(tableDeal, tablep);

  if (res!=1)
    return res;

  res=Par(tablep, presp, vulnerable);

  return res;

}
#endif

int STDCALL Par(struct ddTableResults * tablep, struct parResults *presp, 
	int vulnerable) {
       /* vulnerable 0: None  1: Both  2: NS  3: EW */

	/* The code for calculation of par score / contracts is based upon the
	perl code written by Matthew Kidd ACBLmerge. He has kindly given me permission
	to include a C++ adaptation in DDS. */

	/* The Par function computes the par result and contracts. */


  int denom_conv[5] = { 4, 0, 1, 2, 3 };
  /* Preallocate for efficiency. These hold result from last direction
     (N-S or E-W) examined. */
  int i, j, k, m, isvul;
  int current_side, both_sides_once_flag, denom_max, max_lower;
  int new_score_flag, sc1, sc2, sc3;
  int prev_par_denom = 0, prev_par_tricks = 0;
  int denom_filter[5] = { 0, 0, 0, 0, 0 };
  int no_filtered[2] = { 0, 0 };
  int no_of_denom[2];
  int best_par_score[2];
  int best_par_sacut[2];
  struct best_par_type best_par[5][2];	/* 1st index order number. */

  int ut, t1, t2, tt, score, dr, tu, tu_max, t3[5], t4[5], n;
  struct par_suits_type par_suits[5];
  char contr_sep[2] = { ',', '\0' };
  char temp[8], buff[4];

  int par_denom[2] = { -1, -1 };	 /* 0-4 = NT,S,H,D,C */
  int par_tricks[2] = { 6, 6 };	 /* Initial "contract" beats 0 NT */
  int par_score[2] = { 0, 0 };
  int par_sacut[2] = { 0, 0 };     /* Undertricks for sacrifice (0 if not sac) */

  int rawscore(int denom, int tricks, int isvul);
  void IniSidesString(int dr, int i, int t1, int t2, char stri[]);
  int CalcMultiContracts(int max_lower, int tricks);
  int VulnerDefSide(int side, int vulnerable);

  /* Find best par result for N-S (i==0) or E-W (i==1). These will
     nearly always be the same, but when we have a "hot" situation
     they will not be. */

  for (i = 0; i <= 1; i++) {
  /* Start with the with the offensive side (current_side = 0) and alternate
  between sides seeking the to improve the result for the current side.*/

    no_filtered[i] = 0;
    for (m = 0; m <= 4; m++)
      denom_filter[m] = 0;

    current_side = 0;  both_sides_once_flag = 0;
    while (1) {

    /* Find best contract for current side that beats current contract.
    Choose highest contract if results are equal. */

      k = (i + current_side) % 2;

      isvul = ((vulnerable == 1) || (k ? (vulnerable == 3) : (vulnerable == 2)));

      new_score_flag = 0;
      prev_par_denom = par_denom[i];
      prev_par_tricks = par_tricks[i];

      /* Calculate tricks and score values and
      store them for each denomination in structure par_suits[5]. */

      n = 0;
      for (j = 0; j <= 4; j++) {
        if (denom_filter[j] == 0) {
	  /* Current denomination is not filtered out. */
	  t1 = k ? tablep->resTable[denom_conv[j]][1] : tablep->resTable[denom_conv[j]][0];
	  t2 = k ? tablep->resTable[denom_conv[j]][3] : tablep->resTable[denom_conv[j]][2];
	  tt = Max(t1, t2);
	  /* tt is the maximum number of tricks current side can take in
	  denomination.*/

	  par_suits[n].suit = j;
	  par_suits[n].tricks = tt;

	  if ((tt > par_tricks[i]) || ((tt == par_tricks[i]) &&
	     (j < par_denom[i])))
	    par_suits[n].score = rawscore(j, tt, isvul);
	  else
	    par_suits[n].score = rawscore(-1, prev_par_tricks - tt, isvul);
	  n++;
	}
      }

      /* Sort the items in the par_suits structure with decreasing order of the
	 values on the scores. */

      for (int s = 1; s < n; s++) {
	struct par_suits_type tmp = par_suits[s];
	int r = s;
	for (; r && tmp.score > par_suits[r - 1].score; --r)
	  par_suits[r] = par_suits[r - 1];
	par_suits[r] = tmp;
      }

      /* Do the iteration as before but now in the order of the sorted denominations. */

      for (m = 0; m<n; m++) {
	j = par_suits[m].suit;
	tt = par_suits[m].tricks;

	if ((tt > par_tricks[i]) || ((tt == par_tricks[i]) &&
	   (j < par_denom[i]))) {
	   /* Can bid higher and make contract.*/
	  score = rawscore(j, tt, isvul);
	}
	else {
	 /* Bidding higher in this denomination will not beat previous denomination
	    and may be a sacrifice. */
	  ut = prev_par_tricks - tt;
	  if (j >= prev_par_denom) {
	    /* Sacrifices higher than 7N are not permitted (but long ago
	       the official rules did not prohibit bidding higher than 7N!) */
	    if (prev_par_tricks == 13)
	      continue;
	    /* It will be necessary to bid one level higher, resulting in
             one more undertrick. */
	    ut++;
	  }
	  /* Not a sacrifice (due to par_tricks > prev_par_tricks) */
	  if (ut <= 0)
	    continue;
	  /* Compute sacrifice.*/
	  score = rawscore(-1, ut, isvul);
	}

	if (current_side == 1)
	  score = -score;

	if (((current_side == 0) && (score > par_score[i])) ||
	   ((current_side == 1) && (score < par_score[i]))) {
	  new_score_flag = 1;
	  par_score[i] = score;
	  par_denom[i] = j;

	  if (((current_side == 0) && (score > 0)) ||
		 ((current_side == 1) && (score < 0))) {
	    /* New par score from a making contract.
	    Can immediately update since score at same level in higher
	    ranking suit is always >= score in lower ranking suit and
	    better than any sacrifice. */

	    par_tricks[i] = tt;
	    par_sacut[i] = 0;
	  }
	  else {
	    par_tricks[i] = tt + ut;
	    par_sacut[i] = ut;
	  }
	}
      }


      if (!new_score_flag && both_sides_once_flag) {
	if (no_filtered[i] == 0) {
	  best_par_score[i] = par_score[i];
	  best_par_sacut[i] = par_sacut[i];
	  no_of_denom[i] = 0;
	}
	else if (best_par_score[i] != par_score[i])
	  break;
	if (no_filtered[i] >= 5)
	  break;
	denom_filter[par_denom[i]] = 1;
	no_filtered[i]++;
	best_par[no_of_denom[i]][i].par_denom = par_denom[i];
	best_par[no_of_denom[i]][i].par_tricks = par_tricks[i];
	no_of_denom[i]++;
	both_sides_once_flag = 0;
	current_side = 0;
	par_denom[i] = -1;
	par_tricks[i] = 6;
	par_score[i] = 0;
	par_sacut[i] = 0;     
      }
      else {
	both_sides_once_flag = 1;
	current_side = 1 - current_side;
      }
    }
  }

  presp->parScore[0][0] = 'N';
  presp->parScore[0][1] = 'S';
  presp->parScore[0][2] = ' ';
  presp->parScore[0][3] = '\0';
  presp->parScore[1][0] = 'E';
  presp->parScore[1][1] = 'W';
  presp->parScore[1][2] = ' ';
  presp->parScore[1][3] = '\0';

  sprintf(temp, "%d", best_par_score[0]);
  strcat(presp->parScore[0], temp);
  sprintf(temp, "%d", best_par_score[1]);
  strcat(presp->parScore[1], temp);

  presp->parContractsString[0][0] = 'N';
  presp->parContractsString[0][1] = 'S';
  presp->parContractsString[0][2] = ':';
  presp->parContractsString[0][3] = '\0';
  presp->parContractsString[1][0] = 'E';
  presp->parContractsString[1][1] = 'W';
  presp->parContractsString[1][2] = ':';
  presp->parContractsString[1][3] = '\0';      

  if (best_par_score[0] == 0) {
    /* Neither side can make anything.*/
    return 1;
  }

  for (i = 0; i <= 1; i++) {

    if (best_par_sacut[i] > 0) {
      /* Sacrifice */
      dr = (best_par_score[i] > 0) ? 0 : 1;
      /* Sort the items in the best_par structure with increasing order of the
      values on denom. */
				
      for (int s = 1; s < no_of_denom[i]; s++) {
	struct best_par_type tmp = best_par[s][i];
	int r = s;
	for (; r && tmp.par_denom < best_par[r - 1][i].par_denom; --r)
	   best_par[r][i] = best_par[r - 1][i];
	best_par[r][i] = tmp;
      }

      for (m = 0; m < no_of_denom[i]; m++) {

	j = best_par[m][i].par_denom;

	t1 = ((dr + i) % 2) ? tablep->resTable[denom_conv[j]][0] : tablep->resTable[denom_conv[j]][1];
	t2 = ((dr + i) % 2) ? tablep->resTable[denom_conv[j]][2] : tablep->resTable[denom_conv[j]][3];
	tt = (t1 > t2) ? t1 : t2;

	IniSidesString(dr, i, t1, t2, buff);

	if (presp->parContractsString[i][3] != '\0')
	   strcat(presp->parContractsString[i], contr_sep);

	strcat(presp->parContractsString[i], buff);
	sprintf(temp, "%d", best_par[m][i].par_tricks - 6);
	buff[0] = cardSuit[denom_conv[j]];
	buff[1] = 'x';
	buff[2] = '\0';
	strcat(temp, buff);
	strcat(presp->parContractsString[i], temp);
      }
    }
    else {
      /* Par contract is a makeable contract.*/

      dr = (best_par_score[i] < 0) ? 0 : 1;

      tu_max = 0;
      for (m = 0; m <= 4; m++) {
	t3[m] = ((dr + i) % 2 == 0) ? tablep->resTable[denom_conv[m]][0] : tablep->resTable[denom_conv[m]][1];
	t4[m] = ((dr + i) % 2 == 0) ? tablep->resTable[denom_conv[m]][2] : tablep->resTable[denom_conv[m]][3];
	tu = (t3[m] > t4[m]) ? t3[m] : t4[m];
	if (tu > tu_max) {
	  tu_max = tu;
	  denom_max = m;  /* Lowest denomination if several denominations have max tricks. */
	}
      }

      for (m = 0; m < no_of_denom[i]; m++) {
	j = best_par[m][i].par_denom;

	t1 = ((dr + i) % 2) ? tablep->resTable[denom_conv[j]][0] : tablep->resTable[denom_conv[j]][1];
	t2 = ((dr + i) % 2) ? tablep->resTable[denom_conv[j]][2] : tablep->resTable[denom_conv[j]][3];
	tt = (t1 > t2) ? t1 : t2;

	IniSidesString(dr, i, t1, t2, buff);

	if (presp->parContractsString[i][3] != '\0')
          strcat(presp->parContractsString[i], contr_sep);

	strcat(presp->parContractsString[i], buff);

	if (denom_max < j)
	  max_lower = best_par[m][i].par_tricks - tu_max - 1;
	else
	  max_lower = best_par[m][i].par_tricks - tu_max;

	/* max_lower is the maximal contract lowering, otherwise opponent contract is
	higher. It is already known that par_score is high enough to make
	opponent sacrifices futile.
	To find the actual contract lowering allowed, it must be checked that the
	lowered contract still gets the score bonus points that is present in par score.*/

	sc2 = abs(best_par_score[i]);
	/* Score for making the tentative lower par contract. */
	while (max_lower > 0) {
	  if (denom_max < j)
	    sc1 = -rawscore(-1, best_par[m][i].par_tricks - max_lower - tu_max,
		VulnerDefSide(best_par_score[0]>0, vulnerable));
	  else
	    sc1 = -rawscore(-1, best_par[m][i].par_tricks - max_lower - tu_max + 1,
			VulnerDefSide(best_par_score[0] > 0, vulnerable));
	   /* Score for undertricks needed to beat the tentative lower par contract.*/

	  if (sc2 < sc1)
	    break;
	  else
	    max_lower--;
		
	  /* Tentative lower par contract must be 1 trick higher, since the cost
	  for the sacrifice is too small. */
	}

	int opp_tricks = Max(t3[j], t4[j]);

	while (max_lower > 0) {
	  sc3 = -rawscore(-1, best_par[m][i].par_tricks - max_lower - opp_tricks,
	     VulnerDefSide(best_par_score[0] > 0, vulnerable));

	  /* If opponents to side with par score start the bidding and has a sacrifice
	     in the par denom on the same trick level as implied by current max_lower,
	     then max_lower must be decremented. */

	  if ((sc2 > sc3) && (best_par_score[i] < 0))
	     /* Opposite side with best par score starts the bidding. */
	    max_lower--;
	  else
	    break;
	}

	switch (j) {
	  case 0:  k = 0; break;
	  case 1:  case 2: k = 1; break;
	  case 3:  case 4: k = 2;
	}

	max_lower = Min(max_low[k][best_par[m][i].par_tricks - 6], max_lower);

	n = CalcMultiContracts(max_lower, best_par[m][i].par_tricks);

	sprintf(temp, "%d", n);
	buff[0] = cardSuit[denom_conv[j]];
	buff[1] = '\0';
	strcat(temp, buff);
	strcat(presp->parContractsString[i], temp);
      }
    }
  }

  return 1;
}


int STDCALL SidesPar(struct ddTableResults * tablep, struct parResultsDealer sidesRes[2], int vulnerable) {

  int res, h, hbest[2], k;
  struct parResultsDealer parRes2[4];

  for (h = 0; h <= 3; h++) {

    res = DealerPar(tablep, &parRes2[h], h, vulnerable);

    char * p = strstr(parRes2[h].contracts[0], "pass");
    if (p != NULL) {
      parRes2[h].number = 1;
      parRes2[h].score = 0;
    }
  }

  if (parRes2[2].score > parRes2[0].score)
    hbest[0] = 2;
  else
    hbest[0] = 0;

  if (parRes2[3].score > parRes2[1].score)
    hbest[1] = 3;
  else
    hbest[1] = 1;

  sidesRes[0].number = parRes2[hbest[0]].number;
  sidesRes[0].score = parRes2[hbest[0]].score;
  sidesRes[1].number = parRes2[hbest[1]].number;
  sidesRes[1].score = -parRes2[hbest[1]].score;

  for (k = 0; k < sidesRes[0].number; k++)
    strcpy(sidesRes[0].contracts[k], parRes2[hbest[0]].contracts[k]);

  for (k = 0; k < sidesRes[1].number; k++)
    strcpy(sidesRes[1].contracts[k], parRes2[hbest[1]].contracts[k]);

  return res;
}


int rawscore(int denom, int tricks, int isvul) {
  int game_bonus, level, score;

  /* Computes score for undoubled contract or a doubled contract with
     for a given number of undertricks. These are the only possibilities
     for a par contract (aside from a passed out hand). 
  
     denom  - 0 = NT, 1 = Spades, 2 = Hearts, 3 = Diamonds, 4 = Clubs
             (same order as results from double dummy solver); -1 undertricks
     tricks - For making contracts (7-13); otherwise, number of undertricks.
     isvul  - True if vulnerable */

  if (denom==-1) {
    if (isvul)
      return -300 * tricks + 100;
    if (tricks<=3)
      return -200 * tricks + 100;
    return -300 * tricks + 400;
  }
  else {
    level=tricks-6;
    game_bonus=0;
    if (denom==0) {
      score=10 + 30 * level;
      if (level>=3)
	game_bonus=1;
    }
    else if ((denom==1)||(denom==2)) {
      score=30 * level;
      if (level>=4)
        game_bonus=1;
    }
    else {
      score=20 * level;
      if (level>=5)
	game_bonus=1;
    }
    if (game_bonus) {
      score+= (isvul ? 500 : 300);
    }
    else
      score+=50;

    if (level==6) {
      score+= (isvul ? 750 : 500);
    }
    else if (level==7) {
      score+= (isvul ? 1500 : 1000);
    }
  }

  return score;
}


void IniSidesString(int dr, int i, int t1, int t2, char stri[]) {

   if ((dr+i) % 2 ) {
     if (t1==t2) {
       stri[0]='N';
       stri[1]='S';
       stri[2]=' ';
       stri[3]='\0';
     }
     else if (t1 > t2) {
       stri[0]='N';
       stri[1]=' ';
       stri[2]='\0';
     }
     else {
       stri[0]='S';
       stri[1]=' ';
       stri[2]='\0';
     }
   }
   else {
     if (t1==t2) {
       stri[0]='E';
       stri[1]='W';
       stri[2]=' ';
       stri[3]='\0';
     }
     else if (t1 > t2) {
       stri[0]='E';
       stri[1]=' ';
       stri[2]='\0';
     }
     else {
       stri[0]='W';
       stri[1]=' ';
       stri[2]='\0';
     }
   }
   return;
}


int CalcMultiContracts(int max_lower, int tricks) {
  int n;

  switch (tricks-6) {
    case 5: if (max_lower==3) {n = 2345;}
	    else if (max_lower==2) {n = 345;}
	    else if (max_lower==1) {n = 45;}
	    else {n = 5;}
	    break;
    case 4: if (max_lower==3) {n = 1234;}
	    else if (max_lower==2) {n = 234;}
	    else if (max_lower==1) {n = 34;}
	    else {n = 4;}
	    break;
    case 3: if (max_lower==2) {n = 123;}
	    else if (max_lower==1) {n = 23;}
	    else {n = 3;}
	    break;
    case 2: if (max_lower==1) {n = 12;}
	    else {n = 2;}
	    break;
    default: n = tricks-6;
  }
  return n;
}


int VulnerDefSide(int side, int vulnerable) {
  if (vulnerable == 0)
    return 0;
  else if (vulnerable == 1)
    return 1;
  else if (side) {
    /* N/S makes par contract. */
    if (vulnerable == 2)
      return 0;
    else
      return 1;
  }
  else {
    if (vulnerable == 3)
      return 0;
    else
      return 1;
  }
}


