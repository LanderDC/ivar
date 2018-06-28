#include<iostream>
#include <algorithm>
#include<vector>
#include<fstream>
#include<ctime>

/* Substitution Matrix

   A  T  G  C N
A  1 -1 -1 -1 -1 
T -1  1 -1 -1 -1
G -1 -1  1 -1 -1
C -1 -1 -1  1 -1
N -1 -1 -1 -1  1
*/


const int substitution[5][5] = {
  {1,-1,-1,-1, -1},
  {-1,-1,-1,-1, -1},
  {-1,-1,1,-1, -1},
  {-1,-1,-1,1, -1},
  {-1, -1, -1, -1, 1}
};

const int gap_open = 2;
const int gap_extension = -1;
const int max_read_size = 500;
const int max_adapter_size = 60;

int get_sub_score(char a, char b){
  int i,j;
  switch(a){
  case 'A':
    i = 0;
    break;
  case 'T':
    i = 1;
    break;
  case 'G':
    i = 2;
    break;
  case 'C':
    i = 3;
    break;
  case 'N':
    i = 4;
    break;
  }
  switch(b){
  case 'A':
    j = 0;
    break;
  case 'T':
    j = 1;
    break;
  case 'G':
    j = 2;
    break;
  case 'C':
    j = 3;
    break;
  case 'N':
    i = 4;
    break;
  }
  return substitution[i][j];
}

int get_gap_penalty(int k){
  return k * gap_open; // Linear Gap Penalty
}

void print_matrix(int h[][max_adapter_size], int r, int c, std::string read, std::string adap){
  for(int i =0; i< r;i++){
    if(i == 0){
      std::cout << "    ";
      for(int k =0; k < c-1; k++) std::cout << adap[k] << " ";
      std::cout << "\n";
    }
    for(int j = 0; j < c;j++){
      if(j == 0 && i > 0){
	if(i -1 < read.length())
	  std::cout << read[i-1] << " ";
      }
      if(i == 0 && j == 0)
	std::cout << " " << " ";
      std::cout << h[i][j] << " ";
    }
    std::cout << "\n";
  }
}

// Return Value 1 - Diag, 2->Right, 3->Down, 0-> 0
int*  get_score_cell(int h[][max_adapter_size], int i, int j, std::string read, std::string adap){
  int s[] = {0,0,0};
  int *tmp = new int[2];
  tmp[0] = 0;
  tmp[1] = 0;
  s[0] = h[i-1][j-1] + get_sub_score(read[i-1], adap[j-1]);
  s[1] = h[i-1][j] - get_gap_penalty(1);
  s[2] = h[i][j-1] - get_gap_penalty(1);
  int max = (s[0]>=s[1]) ? s[0] : s[1];
  max = (max>=s[2]) ? max : s[2];
  max = (max > 0) ? max : 0;
  tmp[0] = max;
  if(max == s[0]){
    tmp[1] = 1;
  } else if(max == s[1]){
    tmp[1] = 3;
  } else if(max == s[2]){
    tmp[1] = 2;
  }
  return tmp;
}

void print_alignment(char a[2][max_read_size], int n){
  std::cout << "Alignment: " << std::endl << std::endl;
  for(int j =0;j<2;j++){
    if(j == 0)
      std::cout << "Read: ";
    else if(j == 1)
      std::cout << "Adap: ";
    for(int i = n -1; i >= 0;i--){
      std::cout << a[j][i] << " ";
    }
    std::cout << std::endl;
  }
}

int align_seqs(std::string read, std::string adap){
  int h[max_read_size][max_adapter_size];
  int t[max_read_size][max_adapter_size];
  int m = read.length() + 1,
    n=adap.length()+ 1,
    max_i = 0,
    max_j = 0,
    max_v = -1;
  int *tmp = new int[2];
  // Initialize first column to 0
  for(int i = 0; i < n; i++) h[0][i] = 0;
  // Initialize first row to 0
  for(int i = 0; i < m; i++) h[i][0] = 0;
  // print_matrix(h, m, n, read, adap);
  for(int i = 1; i < m; i++){
    for(int j = 1; j < n; j++){
      tmp = get_score_cell(h, i, j, read, adap);
      h[i][j] = *tmp;
      t[i][j] = *(tmp+1);
      if(*tmp > max_v){
	max_i = i;
	max_j = j;
	max_v = *tmp;
      }
    }
  }
  if(max_v <= adap.length() - get_gap_penalty(1))
    return 0;
  // std::cout << "Score: " << max_v << " ";
  // print_matrix(h, m, n, read, adap);
  // print_matrix(t, m, n, read, adap);
  // Traceback
  int _t, _l, _d, _align_n = 0;
  char _align[2][max_read_size];
  // 1 - Diag, 2->Right, 3->Down
  while(max_v != 0){
    _d = h[max_i-1][max_j-1];
    _l = h[max_i][max_j-1];
    _t = h[max_i-1][max_j];
    switch(t[max_i][max_j]){
    case 1:
      max_v = _d;
      max_i = max_i -1;
      max_j = max_j - 1;
      _align[0][_align_n] = read[max_i];
      _align[1][_align_n] = adap[max_j];
      _align_n++;
      break;
    case 2:
      max_v = _l;
      max_j = max_j - 1;
      _align[0][_align_n] = '-';
      _align[1][_align_n] = read[max_i];
      _align_n++;
      break;
    case 3:
      max_v = _t;
      max_i = max_i - 1;
      _align[0][_align_n] = '-';
      _align[1][_align_n] = adap[max_j];
      _align_n++;
      break;
    case 0:
      max_v = 0;
      break;
    }
  }
  // std::cout << max_i << "," <<max_j << std::endl;
  // print_alignment(_align, _align_n);
  return 0;
}

std::string get_reverse_complement(std::string rev_read){
  char t;
  for(int i = 0;i< rev_read.length();i++){
    switch(rev_read[i]){
    case 'A':
      t = 'T';
      break;
    case 'G':
      t='C';
      break;
    case 'C':
      t='G';
      break;
    case 'T':
      t='A';
      break;
    }
    rev_read[i] = t;
  }
  std::reverse(rev_read.begin(), rev_read.end());
  return rev_read;
}

std::vector<std::string> read_adapters_from_fasta(std::string p){
  std::vector<std::string> adp;
  std::ifstream fin(p);
  std::string line;
  while (std::getline(fin, line)){
    if(line[0] == '>')
      continue;
    adp.push_back(line);
  }
  return adp;
}

int find_adapters_contaminants(std::istream &cin, std::string adp_cntms_file){
  std::vector<std::string> adp = read_adapters_from_fasta(adp_cntms_file);
  std::string line;
  std::string bases;
  std::string qual;
  int ctr = 0;
  while (std::getline(cin, line)){
    if(ctr % 4000 == 0)
      std::cout << "Processed " << (ctr/4) << " reads ..." << std::endl;
    if(ctr % 2 == 0){
      ctr++;
      continue;
    }
    if((ctr+1) % 2 == 0 && (ctr + 1) % 4 == 0 ){
      qual = line;
      // std::cout << bases << std::endl;
      // std::cout << qual << std::endl;
      for(std::vector<std::string>::iterator it = adp.begin(); it != adp.end(); ++it) {
	align_seqs(bases.substr(0, (*it).length()), *it);
      }
    } else {
      bases = line;
    }
    ctr++;
  }
  return 0;
}

int main(int argc, char* argv[]){
  std::string adp = "../data/adapters/NexteraPE-PE.fa";
  clock_t begin = clock();
  find_adapters_contaminants(std::cin, adp);
  clock_t end = clock();
  double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
  std::cout << "Time Taken: " << elapsed_secs << std::endl;
  // std::string b = "CGTAAGTGACAACTTGTCCGCTCCCCCTTTGTTCTTGTCTCTAAATAATGTCCCTAACTGTTTTCCCTTTTTCTTCTTGTCTAAGGACCTTTTCCACTTTGTTTTTTTCTGTGTCCTTCATTTTTGCCATTTCCATTTCCCTTTTCCCTTTCTCCCTTTTTTTGTTTATTAGTGCTTCTTTCTCCATTTCAATCCTCCTCATTCTTCTTTCCCAGCCTTCTGTCTCTTCTTCATTCATCCTTCCTCCTTTT";
  // std::string a = "TCGTCGGCAGCGTCAGATGTGTATAAGAGACAG";
  // align_seqs(b, a);
  return 0;
}
