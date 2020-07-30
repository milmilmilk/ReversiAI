#include <math.h>
#include <float.h>
#include "reversi.h"
#define MAX_DEPTH 3 // 何手先まで読むか
#define MAX_BRANCH ZZ // 何個の子ノードを展開するか
#define DELTA 1e-2

double score[ZZ];
int start_turnum = 0;
double weight[FEATURE_N] = {0.25, 0.25, 0.25, 0.25};

double max_value(node *n, double alpha, double beta);
double min_value(node *n, double alpha, double beta);

int equals(double a, double b){
  return fabs(a - b) <= DBL_EPSILON * fmax(1, fmax(fabs(a), fabs(b)));
}

void print_array(double a[], int len){
  int i;
  printf("["); for(i=0; i<len; i++) printf("%f%s", a[i], (i==len-1)?"]\n":", ");
}

/**
 * @fn
 * シグモイド関数
 * @detail [-∞,∞]を[0,1]にする．
 */
double sigmoid(double x){
  return 1.0 / (1.0 + exp(-x));
}

/**
 * @fn
 * ノードのメモリを解放する
 */
void free_node(node *n){
  node *c;
  while(n != NULL){
    c = n->child;
    free(n);
    n = c;
  }
}

/**
 * @fn
 * score を比較する．qsort で使う．
 */
int cmp_score(const void *a, const void *b){
  return score[*(int*)a] - score[*(int*)b];
}

double max_value(node *n, double alpha, double beta){
  node *c[ZZ];
  int order[ZZ], len, i, j, vi;
  double v, v_now;
  if(n->turnum-start_turnum >= MAX_DEPTH) return sigmoid(eval(n,weight)); // 一定の深さで打ち切る
  //for(i=0; i<n->turnum-start_turnum; i++)printf(" "); print_node_short(n);
  len = children(c, n); // 子ノード
  if(len == 0) return utility(n);
  for(i=0; i<len; i++) order[i] = i;
  for(i=0; i<len; i++) score[i] = sigmoid(eval(c[i],weight)); // 評価
  qsort(order, len, sizeof(int), cmp_score);
  v = min_utility - 1.0;
  for(i=0; i<len && i<MAX_BRANCH; i++){
    v_now = min_value(c[order[len-1-i]], alpha, beta); // 探索
    if(v_now > v){ // 最大値を探す
      v = v_now;
      vi = order[len-1-i];
    }
    if(v > beta){ // beta cut
      for(j=0; j<len; j++) free_node(c[j]);
      return v;
    }
    if(v > alpha) alpha = v;
  }
  n->child = c[vi];
  for(i=0; i<len; i++) if(i != vi) free_node(c[i]);
  return v;
}

double min_value(node *n, double alpha, double beta){
  node *c[ZZ];
  int order[ZZ], len, i, j, vi;
  double v, v_now;
  if(n->turnum-start_turnum >= MAX_DEPTH) return sigmoid(eval(n,weight)); // 一定の深さで打ち切る
  //for(i=0; i<n->turnum-start_turnum; i++)printf(" "); print_node_short(n);
  len = children(c, n); // 子ノード
  if(len == 0) return utility(n);
  for(i=0; i<len; i++) order[i] = i;
  for(i=0; i<len; i++) score[i] = sigmoid(eval(c[i],weight)); // 評価
  qsort(order, len, sizeof(int), cmp_score);
  v = max_utility + 1.0;
  for(i=0; i<len && i<MAX_BRANCH; i++){
    v_now = max_value(c[order[i]], alpha, beta); // 探索
    if(v_now < v){ // 最小値を探す
      v = v_now;
      vi = order[i];
    }
    if(v < alpha){ // alpha cut
      for(j=0; j<len; j++) free_node(c[j]);
      return v;
    }
    if(v < beta) beta = v;
  }
  n->child = c[vi];
  for(i=0; i<len; i++) if(i != vi) free_node(c[i]);
  return v;
}

/**
 * @fn
 * アルファ・ベータ探索
 * @param (n) 入出力． n->child に子孫ノードを格納して返す．
 * @return 利得
 * @detail 全ての子ノードの評価値を計算し，良い順に MAX_BRANCH 個のノードを展開する．また， MAX_DEPTH 手先まで呼んだら打ち切る．打ち切った場合は先端ノードの評価値を返し，終端ノードまでたどり着いたら利得を返す．先端（または終端）ノードの利得が一番良いものを選び, n->child に格納する．
 */
double minimax(node *n){
  start_turnum = n->turnum;
  if(n->turn == 1) return max_value(n, min_utility, max_utility);
  else             return min_value(n, min_utility, max_utility);
}

/**
 * @fn
 * 乱数を使ってゲームをプレイして，結果を返す
 */
double random_value(node *n){
  node *c[ZZ];
  int len, i, vi;
  double scr[ZZ], sumscr=0, r;
  len = children(c, n);
  if(len == 0) return utility(n);
  for(i=0; i<len; i++) sumscr += (scr[i] = sigmoid(eval(c[i],weight)));
  r = (double)rand() / RAND_MAX;
  for(i=0; i<len; i++){
    // r -= 1.0/len;
    r -= (double)scr[i] / sumscr;
    if(r <= 0) break;
  }
  vi = i;
  r = random_value(c[vi]); // 再帰
  n->child = c[vi];
  for(i=0; i<len; i++) if(i != vi) free_node(c[i]);
  return r;
}

/**
 * @fn
 * ロジスティック回帰を行い，重み w を決める．
 */
void learn(int loop){
  node *n, *c;
  int sb, i, j, show=50;
  double minimaxv, wsum, scr, delta=DELTA;
  double w[FEATURE_N];
  // 学習
  srand(0);
  for(i=0; i<show; i++) printf("_"); printf("\n");
  show = loop / show;
  for(i=0; i<loop; i++){
    n = game_start();
    minimaxv = random_value(n);
    sb = equals(minimaxv,1.0)? +1: -1;
    for(j=0; j<FEATURE_N; j++) w[j] = weight[j];
    while(n!=NULL && n->turnum<40){
      // scr = minimaxv - sigmoid(eval(n,weight));
      for(j=0; j<FEATURE_N; j++){
	weight[j] -= DELTA * sb * sigmoid(eval(n,w))*w[j]*feature(j,n);
      }
      wsum = 0;
      for(j=0; j<FEATURE_N; j++) wsum += weight[j];
      for(j=0; j<FEATURE_N; j++) weight[j] /= wsum;
      c = n->child;
      free(n);
      n = c;
    }
    delta *= 0.9;
    if(i%show == 0) printf("#"); fflush(stdout);
    //print_array(weight, FEATURE_N); // 重みを表示
  }
  printf("\n");
}

/**
 * @fn
 * プレイヤは白として，コンピュータと対戦する．
 * @param (w) 重み
 * @return 利得（勝者）
 */
double play_game(){
  int x, y;
  square s;
  node *n;
  int player = 1;
  n = game_start();
  while(1){
    // プレイヤの手番
    if(terminal_test(n)) return utility(n);
    print_node(n);
    if(is_pass(n)){
      n = new_node_pass(n);
    }else{
      do{
	printf("your turn: ");
	scanf("%d %d", &x, &y);
	s = get_square(x,y);
      }while(!placable(n,s));
      n = new_node(n, s);
    }
    
    // コンピュータの手番
    if(terminal_test(n)) return utility(n);
    minimax(n);
    n = n->child;
  }
}

int main(){
  node *n;
  double scr;
  
  if(1){ // 学習
    learn(1000);
    printf("weight = "); print_array(weight, FEATURE_N);
  }

  if(0){ // 探索
    n = game_start();
    scr = minimax(n);
    print_nodes(n);
    free_node(n);
    printf("%f\n", scr);
  }

  if(1){ // ゲームをプレイする
    scr = play_game();
    printf("%s\n", (scr==0)?"BLACK":"WHITE");
  }
  return 0;
}
