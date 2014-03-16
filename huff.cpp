#ifndef __PROGTEST__
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <vector>

using namespace std;
#endif

unsigned int pow2(unsigned int a) {
  if (a == 0) { return 1; }
  else { return 2 * (pow2(a-1)); }
}

#define EMPTY_NODE 0
#define CHAR_NODE 1
#define CHAR_SIZE sizeof(char)*8
#define CPT pow2(CHAR_SIZE-1)

#define UTF8_2   49152
#define UTF8_3   14680064
#define UTF8_4   4026531840
#define UTF8_END 4103110784
#define UTF8_INVALID -5

class bitReader {
  unsigned char actualByte;
  long counter;
  int cursor;
  unsigned char * file;
  long size;

  public:

  bitReader (unsigned char _file[], long _size) {
    file = _file;
    size = _size;
    counter = 0;
    actualByte = file[0];
    cursor = CPT*2;
  }

  unsigned int readBit() {
    // Mv the pointer to the next bit
    cursor /= 2;
    if (cursor == 0) {
      counter++;
      if (counter < size) { actualByte = file[counter]; cursor = CPT; }
      else { throw EOF; }
    }
    return (actualByte & cursor) / cursor;
  }

  unsigned int readBits(unsigned int sob) {
    unsigned int _cursor = pow2(sob-1);
    unsigned int res     = 0;
    while (_cursor > 0) {
      unsigned int actualBit = this->readBit();
      res += _cursor * actualBit;
      _cursor /= 2;
    }

    return res;
  }
};

class hTree {
  struct hNode {
    unsigned int val;
    bool leaf;
    hNode * left, * right;
  };
  hNode * root;
  hNode  * constructor (vector <unsigned int> vect, int * count) {
    if (*count > vect.size()) { return NULL; }
    hNode * n = new hNode;
    if (vect[*count] == 0) {
      n->val  = 0;
      n->leaf = false;
      *count = *count + 1;
      n->left  = constructor(vect, count);
      n->right = constructor(vect, count);
    } else {
      n->val  = vect[*count+1];
      n->leaf = true;
      n->left = n->right = NULL;
      *count = *count+2;
    }

    return n;
  }

  void destructor (hNode * n) {
    if ( n->left )  { destructor(n->left); }
    if ( n->right ) { destructor(n->right); }

    delete n;
  }

  void display (hNode * n) {
    cout << n->val << "|" << n->leaf << endl;
    if (n->left) {
      cout << "l : ";
      display(n->left);
    }
    if (n->right) {
      cout << "r : ";
      display(n->right);
    }
  }

  public:

  hTree (vector <unsigned int> vect) {
    int count = 0;
    root = constructor (vect,&count);
  }

  ~hTree () {
    if (root)
      destructor (root);
  }

  vector <unsigned char> eval(bitReader &file) {
    hNode * n = root;
    unsigned int curs = file.readBit();
    vector <unsigned char> res;

    while (true) {
      if (curs == 0) {
        n = n->left;
      } else {
        n = n->right;
        cout << n->leaf << endl;
      }

      if (n->leaf) {
        if (n->val != UTF8_END) {
          unsigned int nVal = n->val;
          if (nVal < 256) { res.push_back( (char) nVal ); }
          else if (nVal < 65536) {
            res.push_back( (char) (nVal/256) );
            res.push_back( (char) (nVal%256) );
          } else if (nVal < 16777216) {
            res.push_back( (char) (nVal/65536) );
            res.push_back( (char) ( (nVal%65536)/256) );
            res.push_back( (char) ( (nVal%65536)%256) );
          } else {
            res.push_back( (char) (nVal/16777216) );
            res.push_back( (char) ( (nVal%16777216)/65536) );
            res.push_back( (char) ( ((nVal%16777216)%65536)/256) );
            res.push_back( (char) ( ((nVal%16777216)%65536)%256) );
          }
          n = root;
        } else { break; }
      }

      if (n == NULL || n->left == NULL || n->right == NULL) {
        cout << "WTF!" << endl;
        throw -9;
      }

      try {
        curs = file.readBit();
      } catch (int e) {
        if (e == EOF) { break; }
        else { cout << "Error: " << e << endl; break; }
      }
    }

    return res;
  }

  void display () {
    display (root);
  }
};

vector <unsigned int> extractBinTree (bitReader &file) {
  unsigned int actualNb = file.readBit();
  vector <unsigned int> res;
  while (true) {
    res.push_back(actualNb);
    if (actualNb == 1) {
      if (file.readBit() == 0) { res.push_back( file.readBits(7) ); }
      else if (file.readBits(2) == 2) { res.push_back( file.readBits(13) + UTF8_2); }
      else if (file.readBit() == 0) { res.push_back( file.readBits(20) + UTF8_3 );}
      else if (file.readBit() == 0) {
        actualNb = file.readBits(27) + UTF8_4;
        res.push_back(actualNb);
        if (actualNb == UTF8_END) { break; }
      } else { throw UTF8_INVALID; }
    }

    actualNb = file.readBit();
  }
  return res;
}

bool compressFile ( const char * inFile, const char * outFile ) {
  return false;
}

bool decompressFile ( const char * inFile, const char * outFile ) {

  FILE * input  = fopen(inFile , "rb");
  if (input == NULL) {
    perror ( "Error when opening the file");
    return false;
  }

  FILE * output = fopen(outFile, "wb");
  if (input == NULL) {
    perror ( "Error when writing the file" );
    return false;
  }

  fseek(input, 0, SEEK_END);
  long fsize = ftell(input);
  fseek(input, 0, SEEK_SET);
  unsigned char * allFile = new unsigned char [fsize];
  fread(allFile, fsize, 1, input);
  fclose (input);
  bitReader inputBits (allFile, fsize);

  vector <unsigned int> huffTreeVect = extractBinTree(inputBits);
  //for (int i=0; i < huffTreeVect.size(); i++) { cout<<huffTreeVect[i]<<" "; } cout<< endl<<endl;
  hTree * huffTree = new hTree (huffTreeVect);
  vector <unsigned char> out = huffTree->eval(inputBits);
  fwrite (&out[0], sizeof(unsigned char), out.size(), output);
  fclose(output);
  delete huffTree;
  delete [] allFile;


  return true;
}

#ifndef __PROGTEST__
int main ( int argc, char * argv [] ) {

  decompressFile(argv[1],argv[2]);
  return 0;
}
#endif
