#ifndef _REVERSI_H_
#define _REVERSI_H_

#include <stdio.h>
#include <stdlib.h>
#define Z 8 // マスの一列 // 偶数
#define ZZ Z*Z
#define PASS 64
#define DRAW 2
typedef unsigned char byte;
typedef int square;
typedef unsigned long long bitdata;
typedef struct board{
  bitdata exist; // あるかどうか
  bitdata stone; // 白かどうか
  square place; // 置いたとこ
  byte turn; // どっちのターンか
  byte turnum; // 何ターン目か
  byte revnum; // 裏返した数
  struct board *parent;
  struct board *child;
} node;

FILE *fd;
double max_utility = 1.0;
double min_utility = 0.0;

square get_square(byte i, byte j){
  return (square)((i * Z) + j);
}

byte get_bit(bitdata v, square s){
  return (byte)((v >> s) & 1);
}

bitdata set_bit(bitdata v, square s, int b){
  if(b){
    return ((bitdata)1 << s) | v;
  }else{
    return (~((bitdata)1 << s)) & v;
  }
}

///////////
// -1: なし
// 0: 黒
// 1: 白
///////////
char square_char(node *n, square s){
  if(n->place == s){
    return get_bit(n->stone, s)? 'I': 'O';
  }else if(get_bit(n->exist, s)){
    return get_bit(n->stone, s)? '1': '0';
  }else{
    return '-';
  }
}

/**
 * @fn
 * 盤面を表示する．
 * @param (n) 盤面
 */
void print_node(node *n){
  byte i, j;
  if(n == NULL) printf("{NULL}\n");
  printf("%d: ", n->turnum);
  printf("%d", !(n->turn));
  if(n->place==PASS) printf("PASS");
  else printf("(%d,%d)", n->place/Z, n->place%Z);
  printf(" /%d", n->revnum);
  printf("{\n");
  printf("\t  "); for(j=0; j<Z; j++)printf("%d",j); printf("\n");
  for(i=0; i<Z; i++){
    printf("\t%d ",i);
    for(j=0; j<Z; j++){
      printf("%c", square_char(n, get_square(i,j)) );
    }
    printf("\n");
  }
  printf("}\n");
}

/**
 * @fn
 * 盤面を一行で表示する．
 * @param (n) 盤面
 */
void print_node_short(node *n){
  byte i, j;
  if(n == NULL) printf("{NULL}\n");
  printf("%d: ", n->turnum);
  printf("%d", !(n->turn));
  if(n->place==PASS) printf("PASS");
  else printf("(%d,%d)", n->place/Z, n->place%Z);
  printf("\n");
}

/**
 * @fn
 * 手順を表示する．
 * @param (n) 盤面
 */
void print_nodes(node *n){
  while(n->child != NULL){
    print_node_short(n);
    n = n->child;
  }
  print_node(n);
}

void fput_bitdata(bitdata d, FILE *fd){
  square i;
  for(i=56; i<=56; i-=8){ // アンダーフローしてi>56になる
    fputc((unsigned char)(d >> i), fd);
  }
}

/*
 * @fn
 * 盤面をファイルに書き出す．
 * @param (n) 盤面
 *
void write_node(node *n){
  fput_bitdata(n->exist, fd);
  fput_bitdata(n->stone, fd);
  fputc(n->place, fd);
  fputc(n->turn, fd);
  fputc(n->turnum, fd);
  fputc(n->revnum, fd);
  return;
}
*/

int fget_bitdata(bitdata *out, FILE *fd){
  int i;
  bitdata c, d=0;
  for(i=0; i<8; i++){
    if((c = fgetc(fd)) == EOF) return 1;
    d = (d << 8) | c;
  }
  *out = d;
  return 0;
}

/*
 * @fn
 * 盤面をファイルから読み込む．
 * @param (out) 盤面
 *
int read_node(node *out){
  bitdata c;
  if(fget_bitdata(&(out->exist), fd)) return 1;
  if(fget_bitdata(&(out->stone), fd)) return 2;
  if((out->place  =fgetc(fd)) == EOF) return 3;
  if((out->turn   =fgetc(fd)) == EOF) return 4;
  if((out->turnum =fgetc(fd)) == EOF) return 5;
  if((out->revnum =fgetc(fd)) == EOF) return 6;
  return 0;
}
*/

/**
 * @fn
 * 盤面 n でマス s に置けるかどうか
 * @param (n) 盤面
 * @param (s) 置くマス
 * @return 置ければ 1 ，置けなければ 0
 */
int placable_turn(node *n, square s, byte turn){
  square t;
  square sZ;
  if(get_bit(n->exist, s)) return 0;
  for(t=s; t<ZZ; t+=Z){
    if(t == s) continue;
    if(!get_bit(n->exist, t)) break;
    if(get_bit(n->stone, t) == turn){
      if(t == s+Z) break;
      return 1;
    }
  }
  for(t=s; t>=0; t-=Z){
    if(t == s) continue;
    if(!get_bit(n->exist, t)) break;
    if(get_bit(n->stone, t) == turn){
      if(t == s-Z) break;
      return 1;
    }
  }
  sZ = s / Z;
  for(t=s; t/Z == sZ; t+=1){
    if(t == s) continue;
    if(!get_bit(n->exist, t)) break;
    if(get_bit(n->stone, t) == turn){
      if(t == s+1) break;
      return 1;
    }
  }
  for(t=s; t/Z == sZ; t-=1){
    if(t == s) continue;
    if(!get_bit(n->exist, t)) break;
    if(get_bit(n->stone, t) == turn){
      if(t == s-1) break;
      return 1;
    }
  }
  for(t=s; t<ZZ && (t%Z!=0 || t==s); t+=Z+1){
    if(t == s) continue;
    if(!get_bit(n->exist, t)) break;
    if(get_bit(n->stone, t) == turn){
      if(t == s+Z+1) break;
      return 1;
    }
  }
  for(t=s; t<ZZ && (t%Z!=Z-1 || t==s); t+=Z-1){
    if(t == s) continue;
    if(!get_bit(n->exist, t)) break;
    if(get_bit(n->stone, t) == turn){
      if(t == s+Z-1) break;
      return 1;
    }
  }
  for(t=s; t>=0 && (t%Z!=0 || t==s); t-=Z-1){
    if(t == s) continue;
    if(!get_bit(n->exist, t)) break;
    if(get_bit(n->stone, t) == turn){
      if(t == s-(Z-1)) break;
      return 1;
    }
  }
  for(t=s; t>=0 && (t%Z!=Z-1 || t==s); t-=Z+1){
    if(t == s) continue;
    if(!get_bit(n->exist, t)) break;
    if(get_bit(n->stone, t) == turn){
      if(t == s-(Z+1)) break;
      return 1;
    }
  }
  return 0;
}
int placable(node *n, square s) {return placable_turn(n, s, n->turn);}

int is_pass(node *n){
  square t;
  for(t=0; t<ZZ; t++) if(placable(n, t)) return 0;
  return 1;
}

int reversable(node *n, node *o, square s){
  int able = 0;
  square t;
  square sZ = s / Z;
  if(get_bit(o->exist, s)) return 0;
  for(t=s+Z; t<ZZ; t+=Z){
    if(!get_bit(o->exist, t)) break;
    if(get_bit(o->stone, t) == o->turn){
      for(t=t-Z; t!=s; t-=Z){
	n->stone = set_bit(n->stone, t, o->turn);
	able ++;
      }
      break;
    }
  }
  for(t=s-Z; t>=0; t-=Z){
    if(!get_bit(o->exist, t)) break;
    if(get_bit(o->stone, t) == o->turn){
      for(t=t+Z; t!=s; t+=Z){
	n->stone = set_bit(n->stone, t, o->turn);
	able ++;
      }
      break;
    }
  }
  for(t=s+1; t/Z == sZ; t+=1){
    if(!get_bit(o->exist, t)) break;
    if(get_bit(o->stone, t) == o->turn){
      for(t=t-1; t!=s; t-=1){
	n->stone = set_bit(n->stone, t, o->turn);
	able ++;
      }
      break;
    }
  }
  for(t=s-1; t/Z == sZ; t-=1){
    if(!get_bit(o->exist, t)) break;
    if(get_bit(o->stone, t) == o->turn){
      for(t=t+1; t!=s; t+=1){
	n->stone = set_bit(n->stone, t, o->turn);
	able ++;
      }
      break;
    }
  }
  for(t=s+Z+1; t<ZZ && t%Z!=0; t+=Z+1){
    if(!get_bit(o->exist, t)) break;
    if(get_bit(o->stone, t) == o->turn){
      for(t=t-(Z+1); t!=s; t-=Z+1){
	n->stone = set_bit(n->stone, t, o->turn);
	able ++;
      }
      break;
    }
  }
  for(t=s+Z-1; t<ZZ && t%Z!=Z-1; t+=Z-1){
    if(!get_bit(o->exist, t)) break;
    if(get_bit(o->stone, t) == o->turn){
      for(t=t-(Z-1); t!=s; t-=Z-1){
	n->stone = set_bit(n->stone, t, o->turn);
	able ++;
      }
      break;
    }
  }
  for(t=s-(Z-1); t>=0 && t%Z!=0; t-=Z-1){
    if(!get_bit(o->exist, t)) break;
    if(get_bit(o->stone, t) == o->turn){
      for(t=t+Z-1; t!=s; t+=Z-1){
	n->stone = set_bit(n->stone, t, o->turn);
	able ++;
      }
      break;
    }
  }
  for(t=s-(Z+1); t>=0 && t%Z!=Z-1; t-=Z+1){
    if(!get_bit(o->exist, t)) break;
    if(get_bit(o->stone, t) == o->turn){
      for(t=t+Z+1; t!=s; t+=Z+1){
	n->stone = set_bit(n->stone, t, o->turn);
	able ++;
      }
      break;
    }
  }
  return able;
}

/**
 * @fn
 * 盤面 o でマス s に置いた時の，次のノード
 * @param (o) 古い盤面
 * @param (s) 置くマス
 * @return 新しい盤面
 */
node* new_node(node *o, square s){
  node *n = (node*) malloc(sizeof(node));
  n->exist = set_bit(o->exist, s, 1);
  n->stone = set_bit(o->stone, s, o->turn);
  n->revnum = reversable(n, o, s);
  n->turnum = o->turnum + 1;
  n->place = s;
  n->turn = !(o->turn);
  n->parent = o;
  n->child = NULL;
  return n;
}

/**
 * @fn
 * 盤面 o でパスした時の，次のノード
 * @param (o) 古い盤面
 * @return 新しい盤面
 */
node* new_node_pass(node *o){
  node *n = (node*) malloc(sizeof(node));
  n->exist = o->exist;
  n->stone = o->stone;
  n->revnum = 0;
  n->turnum = o->turnum + 1;
  n->place = PASS;
  n->turn = !(o->turn);
  n->parent = o;
  n->child = NULL;
  return n;
}

/**
 * @fn
 * どちらが勝ったか
 * @param (n) 盤面
 * @return 黒が勝ちなら 0 ，白が勝ちなら 1 ，引き分けなら DRAW
 */
int win(node *n){
  bitdata e = n->exist;
  bitdata s = n->stone;
  int num=0, par=0;
  for(; s>0; s=s>>1, e=e>>1){
    if(e & 1){
      par ++;
      if(s & 1) num ++;
    }
  }
  if(num == par/2) return DRAW; // 引き分け
  return !(num < par/2);
}

/**
 * @fn
 * 1 手目の盤面のノードを返す
 * @return 1 手目の盤面のノード
 */
node* game_start(){
  node *n = (node*) malloc(sizeof(node));
  n->exist = (((bitdata)0b11 << Z) + 0b11) << (Z*(Z/2-1)+Z/2-1);
  n->stone = (((bitdata)0b10 << Z) + 0b01) << (Z*(Z/2-1)+Z/2-1);
  n->place = PASS;
  n->turn = 0;
  n->turnum = 0;
  n->revnum = 0;
  n->parent = NULL;
  n->child = NULL;
  
  n = new_node(n, (Z*(Z/2-2)+Z/2-1));
  return n;
}

/**
 * @fn
 * 新しいノードを作る
 * @return 新しい盤面
 */
node* create_node(bitdata exist_b, bitdata stone_b, byte turn_b){
  square t;
  node *n = (node*) malloc(sizeof(node));
  n->exist = exist_b;
  n->stone = stone_b;
  n->place = PASS;
  n->turnum = 0;
  for(t=0; t<ZZ; t++) if(get_bit(n->exist,t)==1) n->turnum++;
  if(turn_b==0 || turn_b==1) n->turn = turn_b;
  else n->turn = n->turnum % 2;
  n->revnum = 0;
  n = new_node(n, (Z*(Z/2-2)+Z/2-1));
  n->parent = NULL;
  n->child = NULL;
  return n;
}

//******************************************************************

#define FEATURE_N 4
/**
 * @fn
 * 特徴
 */
double feature(int i, node *n){
  square t;
  int score;
  switch(i){
  case -1: // 裏返した個数 [0:3*Z] // 前の局面に影響されるため除外
    score = n->revnum;
    if(n->turn==1) return -score;
    else return score;

  case 0: // 白石の個数 - 黒石の個数 [-ZZ:ZZ];
    score = 0;
    for(t=0; t<ZZ; t++){
      score += get_bit(n->exist,t)? 0: (get_bit(n->stone,t)==0? -1: 1);
    }
    return score;

  case 1: // 白が置ける場所の数 - 黒が置ける場所の数 [-ZZ:ZZ];
    score = 0;
    for(t=0; t<ZZ; t++){
      if(placable_turn(n,t,0)) score ++;
      if(placable_turn(n,t,1)) score --;
    }
    return score;

  case 2: // 隅をとった数 [-4:4]
    score = 0;
    score += get_bit(n->exist,0   )? 0: (get_bit(n->stone,0   )==0? -1: 1);
    score += get_bit(n->exist,Z-1 )? 0: (get_bit(n->stone,Z-1 )==0? -1: 1);
    score += get_bit(n->exist,ZZ-Z)? 0: (get_bit(n->stone,ZZ-Z)==0? -1: 1);
    score += get_bit(n->exist,ZZ-1)? 0: (get_bit(n->stone,ZZ-1)==0? -1: 1);
    return score;

  case 3: // マス「 X 」をとった数 [-4:4]
    score = 0;
    score += get_bit(n->exist,Z+1     )? 0: (get_bit(n->stone,Z+1     )==1? -1: 1);
    score += get_bit(n->exist,Z*2-2   )? 0: (get_bit(n->stone,Z*2-2   )==1? -1: 1);
    score += get_bit(n->exist,ZZ-Z*2+1)? 0: (get_bit(n->stone,ZZ-Z*2+1)==1? -1: 1);
    score += get_bit(n->exist,ZZ-Z-2  )? 0: (get_bit(n->stone,ZZ-Z-2  )==1? -1: 1);
    return score;
    
  default:
    return 0;
  }
}

/**
 * @fn
 * 評価関数
 * @param (n) ノード
 * @param (w) 重み
 * @return 評価値 [-∞,∞]
 */
double eval(node *n, double w[]){
  int i;
  double score = 0;
  if(n->turnum < 62){
    // 序盤
    for(i=0; i<FEATURE_N; i++){
      score += w[i] * feature(i, n);
    }
    return score;
    
  }else{
    // 終盤
    // 石の数を数える
    bitdata e = n->exist;
    bitdata s = n->stone;
    int score = 0;
    for(; s>0; s=s>>1, e=e>>1) if((s & 1 & e) == 1) score ++;
    return (double)score;
  }
}

/**
 * @fn
 * 利得
 * @param (n) ノード
 * @return 利得．黒が勝ちなら 0 ，白が勝ちなら 1 ，引き分けなら 0.5 を返す．
 */
double utility(node *n){
  int w;
  //print_node(n);
  w = win(n);
  if(w == DRAW) return 0.5;
  else return (double) w;
}

/**
 * @fn
 * 子ノードを返す
 * @param (out) 出力．子ノードのポインタの配列．
 * @param (n) ノード
 * @return 子ノードの数．
 */
int children(node *out[], node *n){
  square t;
  int count=0;
  for(t=0; t<ZZ; t++){
    if(placable(n, t)){
      out[count++] = new_node(n, t);
    }
  }
  if(count == 0){
    out[0] = new_node_pass(n);
    for(t=0; t<ZZ; t++){
      if(placable(out[0], t)){
	return 1;
      }
    }
    return 0;
  }
  else return count;
}

/**
 * @fn
 * 終端ノードかどうか
 * @param (n) ノード
 * @return 終端ノードなら 1 ，そうでないなら 0 を返す．
 * @detail children を使うなら，「 children(NULL,n)==0 」のほうが処理が少なくて済む．
 */
// int terminal_test(node *n) {return children(NULL,n)==0;}
int terminal_test(node *n) {
  square t;
  node *m;
  for(t=0; t<ZZ; t++) if(placable(n,t)) return 0;
  m = new_node_pass(n);
  for(t=0; t<ZZ; t++) if(placable(m,t)) return 0;
  return 1;
}

#endif
