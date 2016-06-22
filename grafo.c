#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <graphviz/cgraph.h>
#include "grafo.h"
#include <malloc.h>
 
// tamanho máximo da string para representar o peso de uma aresta/arco
#define MAX_STRING_SIZE 256
//------------------------------------------------------------------------------
//ESTRUTURAS
//------------------------------------------------------------------------------
//LISTA
//------------------------------------------------------------------------------
// nó de lista encadeada cujo conteúdo é um void *

struct no {
  void *conteudo;
  no proximo;
};
//------------------------------------------------------------------------------
// lista encadeada

struct lista {
  unsigned int tamanho;
  int padding; // só pra evitar warning 
  no primeiro;
};
//------------------------------------------------------------------------------
//GRAFO
//------------------------------------------------------------------------------
// (apontador para) estrutura de dados para representar um grafo
//
// o grafo pode ser
// - direcionado ou não
// - com pesos nas arestas ou não
//
// além dos vértices e arestas, o grafo tem um nome, que é uma "string"
//
// num grafo com pesos nas arestas todas as arestas tem peso, que é um long int
//
// o peso default de uma aresta é 0
struct grafo{
    char *nome; //nome do grafo
    int direcionado; // 1, se o grafo é direcionado, 0 se não é
    int ponderado; // 1, se o grafo tem pesos nas arestas/arcos, 0 se não é
    unsigned int n_vertices; //numero de vertices
    unsigned int n_arestas; //numero de arestas
    vertice *vertices; //apontador para a estrutura de vertices
};
//------------------------------------------------------------------------------
//VÉRTICE
//------------------------------------------------------------------------------
// (apontador para) estrutura de dados que representa um vértice do grafo
//
// cada vértice tem um nome que é uma "string"
struct vertice{
    char *nome; // nome do vertice
    unsigned int id; // id = posição do vertice no vetor de vertices do grafo, serve para facilitar a busca de vertices
    unsigned int grau_entrada; // grau do vertice
    unsigned int grau_saida; // grau do vertice
    int padding;
    unsigned int *rotulo; //rotulo do vertice {1..n}
    int removido; // se for 1 a aresta do grafo foi removida, se for 0, nao
    int tamRotulo; //tamanho do vetor de rotulos atual 
    int passado; //se foi passado 1, senao 0
    int visitado; //se o vertice foi visitado, muda para 1, senão permanece em 0
    int coberto; // se o vertice esta conectado com uma aresta, muda para 1, senão permanece em zero
    int inSet; //se o vertice ja pertence a um conjunto, muda para 1, senão permanece em zero
    lista adjacencias_entrada;
    lista adjacencias_saida;
};
//------------------------------------------------------------------------------
// estrutura dos vizinhos
typedef struct adjacencia{
    long int peso;
    vertice v_origem; //vertice de origem
    vertice v_destino; //vertice de destino
} *adjacencia;
//------------------------------------------------------------------------------
//CÓDIGOS
//------------------------------------------------------------------------------
//LISTA
//------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// devolve o número de nós da lista l

unsigned int tamanho_lista(lista l) { return l->tamanho; }

//---------------------------------------------------------------------------
// devolve o primeiro nó da lista l,
//      ou NULL, se l é vazia

no primeiro_no(lista l) { return l->primeiro; }

//---------------------------------------------------------------------------
// devolve o conteúdo do nó n
//      ou NULL se n = NULL 

void *conteudo(no n) { return n->conteudo; }

//---------------------------------------------------------------------------
// devolve o sucessor do nó n,
//      ou NULL, se n for o último nó da lista

no proximo_no(no n) { return n->proximo; }

//---------------------------------------------------------------------------
// cria uma lista vazia e a devolve
//
// devolve NULL em caso de falha

lista constroi_lista(void) {

  lista l = malloc(sizeof(struct lista));

  if ( ! l ) 
    return NULL;

  l->primeiro = NULL;
  l->tamanho = 0;

  return l;
  free(l);
}
//---------------------------------------------------------------------------
// desaloca a lista l e todos os seus nós
// 
// se destroi != NULL invoca
//
//     destroi(conteudo(n)) 
//
// para cada nó n da lista. 
//
// devolve 1 em caso de sucesso,
//      ou 0 em caso de falha

int destroi_lista(lista l, int destroi(void *)) { 
  
  no p;
  int ok=1;

  while ( (p = primeiro_no(l)) ) {
    
    l->primeiro = proximo_no(p);

    if ( destroi )
      ok &= destroi(conteudo(p));

    free(p);
  }

  free(l);

  return ok;
}
//---------------------------------------------------------------------------
// insere um novo nó na lista l cujo conteúdo é p
//
// devolve o no recém-criado 
//      ou NULL em caso de falha

no insere_lista(void *conteudo, lista l) { 

  no novo = malloc(sizeof(struct no));

  if ( ! novo ) 
    return NULL;

  novo->conteudo = conteudo;
  novo->proximo = primeiro_no(l);
  ++l->tamanho;
  
  return l->primeiro = novo;
  free(novo);
}

//------------------------------------------------------------------------------
// remove o no de endereço rno de l
// se destroi != NULL, executa destroi(conteudo(rno)) 
// devolve 1, em caso de sucesso
//         0, se rno não for um no de l

int remove_no(struct lista *l, struct no *rno, int destroi(void *)) {
	int r = 1;
	if (l->primeiro == rno) {
		l->primeiro = rno->proximo;
		if (destroi != NULL) {
			r = destroi(conteudo(rno));
		}
		free(rno);
		l->tamanho--;
		return r;
	}
	for (no n = primeiro_no(l); n->proximo; n = proximo_no(n)) {
		if (n->proximo == rno) {
			n->proximo = rno->proximo;
			if (destroi != NULL) {
				r = destroi(conteudo(rno));
			}
			free(rno);
			l->tamanho--;
			return r;
		}
	}
	return 0;
}
//------------------------------------------------------------------------------
//GRAFO E VERTICE
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// cria e devolve um  grafo g
 
static grafo cria_grafo(const char *nome, int direcionado, int ponderado, int total_vertices){
    grafo g = malloc(sizeof(struct grafo)); //aloca memoria pro grafo
 
    if(g == NULL)
    return 0;
   
    g->nome = malloc((strlen(nome) + 1) * sizeof(char));
    strcpy(g->nome, nome);
    g->direcionado = direcionado;
    g->ponderado = ponderado;
    g->n_vertices = 0;
    g->n_arestas = 0;
    g->vertices = malloc((unsigned int)total_vertices* sizeof(vertice));
 
    return g;
    free(g->nome);
    free(g);
}
 
//------------------------------------------------------------------------------
// cria um vizinho e insere na lista de vizinhos do vertice de origem e/ou na de destino tambem
// se ele nao for direcionado
 
static void cria_vizinhanca(grafo g, vertice origem, vertice destino, long int peso){
    adjacencia viz_1 = malloc(sizeof(struct adjacencia));
   
    if(viz_1 == NULL)
    printf("Sem memoria");
    else{
        viz_1->peso = peso;
        viz_1->v_origem = origem;
        viz_1->v_destino = destino;
		insere_lista(viz_1, origem->adjacencias_saida);
        origem->grau_saida++;
        destino->grau_entrada++;
 
        if (!direcionado(g)) {
            // se o grafo não for direcionado, a aresta deve aparecer também na
            // lista de adjacencia do vertice dest
 
            adjacencia viz_2 = malloc(sizeof(struct adjacencia));
            if(viz_2 == NULL)
            printf("Sem memoria");
            else{
                viz_2->peso = peso;
                viz_2->v_origem = destino;
                viz_2->v_destino = origem;
                insere_lista(viz_2, destino->adjacencias_saida);
                destino->grau_saida++;
                origem->grau_entrada++;
                }
        }
    else{
        adjacencia viz_3 = malloc(sizeof(struct adjacencia));
            if(viz_3 == NULL)
            printf("Sem memoria");
            else{
                viz_3->peso = peso;
                viz_3->v_origem = origem;
                viz_3->v_destino = destino;
                insere_lista(viz_3, destino->adjacencias_entrada);
                }
        }
    }
    g->n_arestas++;
}
 
//------------------------------------------------------------------------------
// destroi um vizinho
 
static int destroi_vizinho(adjacencia v){
    free(v);
    return 1;
}
 
//------------------------------------------------------------------------------
// cria, insere no grafo e retorna o vertice
static vertice cria_vertice(grafo g, const char *nome){
    vertice v = malloc(sizeof(struct vertice));
 
    if(v == NULL){
        return 0;
    }else{
        v->id = g->n_vertices;
        v->nome = malloc((strlen(nome) +1) *sizeof(char));
        strcpy(v->nome, nome);
        v->adjacencias_saida = constroi_lista();
        v->adjacencias_entrada = constroi_lista();
        v->grau_entrada = 0;
        v->grau_saida = 0;
        v->removido = 0;
        v->visitado = 0;
        v->coberto = 0;
        v->rotulo = NULL;
        v->inSet = 0;
        g->vertices[v->id] = v;
        g->n_vertices++;

    }
 
    return v;
    free(v->nome);
    free(v);
}
 
//------------------------------------------------------------------------------
// destroi um vertice
static void destroi_vertice(vertice v){
    destroi_lista(v->adjacencias_saida, (int (*)(void *)) destroi_vizinho);
    destroi_lista(v->adjacencias_entrada, (int (*)(void *)) destroi_vizinho);
    v->grau_saida = 0;
    v->grau_entrada = 0;
    free(v->nome);
    free(v);
    v =  NULL;
}
 
//------------------------------------------------------------------------------
// busca um vertice pelo nome do grafo
 
static vertice v_busca(grafo g, char *nome){
    for(unsigned int i = 0; i < g->n_vertices; i++){
        if(strcmp(g->vertices[i]->nome, nome) == 0)
            return g->vertices[i];
    }
    return NULL;
}
 
//------------------------------------------------------------------------------
// devolve o peso de uma aresta no formato libcgraph
 
static long int get_peso(Agedge_t *Ae) {
    char p_str[5];
    strcpy(p_str, "peso");
    char *peso_c = agget(Ae, p_str);
 
    if (peso_c && *peso_c)
        return atol(peso_c);
 
    return 0;
}
 
//------------------------------------------------------------------------------
// devolve 1 se o grafo no formato libcgraph tem pesos nas arestas
 
static int contem_pesos(Agraph_t *Ag) {
    char p_str[5];
    strcpy(p_str, "peso");
 
    for ( Agsym_t *sym = agnxtattr(Ag, AGEDGE, NULL);
        sym;
        sym = agnxtattr(Ag, AGEDGE, sym) ) {
        if (strcmp(sym->name, p_str) == 0)
            return 1;
    }
 
    return 0;
}
 
//------------------------------------------------------------------------------
// devolve o nome do grafo g
 
char *nome_grafo(grafo g){
    char nome_g[1];
    nome_g[0] = '\0';
    return g ? g->nome : nome_g;
}
 
//------------------------------------------------------------------------------
// devolve 1, se g é direcionado, ou
//         0, caso contrário
 
int direcionado(grafo g){
    return g ? g->direcionado : 0;
}
 
//------------------------------------------------------------------------------
// devolve 1, se g tem pesos nas arestas/arcos,
//      ou 0, caso contrário
 
int ponderado(grafo g){
    return g ? g->ponderado : 0;
}
 
//------------------------------------------------------------------------------
// devolve o número de vértices do grafo g
 
unsigned int n_vertices(grafo g){
    return g ? g->n_vertices : 0;
}
 
//------------------------------------------------------------------------------
// devolve o número de arestas/arcos do grafo g
 
unsigned int n_arestas(grafo g){
    return g ? g->n_arestas : 0;
}
 
//------------------------------------------------------------------------------
// devolve o nome do vertice v
 
char *nome_vertice(vertice v){
    char nome_v[1];
    nome_v[0] = '\0';
    return v ? v->nome : nome_v;
}
 
//------------------------------------------------------------------------------
// lê um grafo no formato dot de input, usando as rotinas de libcgraph
//
// desconsidera todos os atributos do grafo lido exceto o atributo
// "peso" quando ocorrer; neste caso o valor do atributo é o peso da
// aresta/arco que é um long int
//
// num grafo com pesos todas as arestas/arcos tem peso
//
// o peso default de uma aresta num grafo com pesos é 0
//
// todas as estruturas de dados alocadas pela libcgraph são
// desalocadas ao final da execução
//
// devolve o grafo lido ou
//         NULL em caso de erro
 
grafo le_grafo(FILE *input){
    if (!input)
        return NULL;
   
    Agraph_t *Ag = agread(input, NULL);
   
    if(!Ag)
        return NULL;
 
    grafo g = cria_grafo(agnameof(Ag), agisdirected(Ag), contem_pesos(Ag), agnnodes(Ag));
 
    for (Agnode_t *Av=agfstnode(Ag); Av; Av=agnxtnode(Ag,Av)) {
            cria_vertice(g, agnameof(Av));
    }
 
        for (Agnode_t *Av=agfstnode(Ag); Av; Av=agnxtnode(Ag,Av)) {
            for (Agedge_t *Ae=agfstout(Ag,Av); Ae; Ae=agnxtout(Ag,Ae)) {
                vertice u = v_busca(g, agnameof(agtail(Ae)));
                vertice v = v_busca(g, agnameof(aghead(Ae)));
                cria_vizinhanca(g, u, v, get_peso(Ae));
            }
        }
   
    agclose(Ag);
    agfree(Ag, NULL);
    return g;
}  
 
//------------------------------------------------------------------------------
// desaloca toda a memória usada em *g
//
// devolve 1 em caso de sucesso ou
//         0 caso contrário
//
// g é um (void *) para que destroi_grafo() possa ser usada como argumento de
// destroi_lista()
 
int destroi_grafo(void *g){
    if (!g)
		return 0;
 
    for (unsigned int i=0; i<((grafo) g)->n_vertices; i++)
        destroi_vertice(((grafo) g)->vertices[i]);
   
    free(((grafo) g)->vertices);
    free(((grafo) g)->nome);
    free(g);
 
    return 1;
}
 
//------------------------------------------------------------------------------
// escreve o grafo g em output usando o formato dot, de forma que
//
// 1. todos os vértices são escritos antes de todas as arestas/arcos
//
// 2. se uma aresta tem peso, este deve ser escrito como um atributo
//    de nome "peso"
//
// devolve o grafo escrito ou
//         NULL em caso de erro
 
grafo escreve_grafo(FILE *output, grafo g){
    if(!g || !output)
        return NULL;
 
    Agraph_t *ag;
    Agsym_t *peso;
 
    char peso_s[MAX_STRING_SIZE];
 
    //criando a string "peso"
    char p_str[5];
    strcpy(p_str, "peso");
 
    //cria uma string vazia pra usar como valor default do atributo peso
 
    char default_s[1];
    default_s[0] = '\0';
 
    if(g->direcionado)
        ag = agopen(g->nome, Agstrictdirected, NULL);
    else
        ag= agopen(g->nome, Agstrictundirected, NULL);
 
    if(g->ponderado)
        peso = agattr(ag, AGEDGE, p_str, default_s);
 
    Agnode_t **nodes = malloc(g->n_vertices * sizeof(Agnode_t*));
   
    for(unsigned int i = 0; i < g->n_vertices; i++)
        nodes[g->vertices[i]->id] = agnode(ag, g->vertices[i]->nome, TRUE);
 
    for(unsigned int i = 0; i < g->n_vertices; i++){
        vertice v = g->vertices[i];
 
        for(no n = primeiro_no(v->adjacencias_saida); n != NULL; n = proximo_no(n)){
            adjacencia viz = conteudo(n);
 
            Agedge_t *ae = agedge(ag, nodes[v->id], nodes[viz->v_destino->id], NULL, TRUE);
               
            if(g->ponderado){
                sprintf(peso_s, "%ld", viz->peso);
                agxset(ae, peso, peso_s);
            }
        }
    }
 
    free(nodes);
    agwrite(ag, output);
    agclose(ag);
    agfree(ag, NULL);
    return g;
}
 
//------------------------------------------------------------------------------
// devolve um grafo igual a g
 
grafo copia_grafo(grafo g){
    if(!g)
        return NULL;
   
    grafo copy = cria_grafo(g->nome, g->direcionado, g->ponderado, (int)g->n_vertices);
 
        if(copy == NULL){
            printf("Sem memoria para alocar.\n");
        }else{
        copy->n_vertices = g->n_vertices;
        copy->n_arestas = g->n_arestas;
 
        for(unsigned int i = 0; i < g->n_vertices; i++){
            copy->vertices[i] = g->vertices[i];
        }
    }
    return copy;
    free(copy);
}
 
//------------------------------------------------------------------------------
// devolve a vizinhança de entrada do vértice v
 
static lista vizinhanca_entrada(vertice v){
    lista viz = constroi_lista();
    for (no n=primeiro_no(v->adjacencias_entrada); n!=NULL; n=proximo_no(n)) {
        adjacencia a = conteudo(n);
        insere_lista(a->v_origem, viz);
    }        
    return viz;
}
 
//------------------------------------------------------------------------------
// devolve a vizinhança de saída do vértice v
 
static lista vizinhanca_saida(vertice v){
 
    lista viz = constroi_lista();
    for (no n=primeiro_no(v->adjacencias_saida); n!=NULL; n=proximo_no(n)) {
        if(n != NULL){
            adjacencia a = conteudo(n);
            insere_lista(a->v_destino, viz);
        }
    }        
    return viz;
}
 
//------------------------------------------------------------------------------
// devolve a vizinhança do vértice v
//
// se direcao == 0, v é um vértice de um grafo não direcionado
//                  e a função devolve sua vizinhanca
//
// se direcao == -1, v é um vértice de um grafo direcionado e a função
//                   devolve sua vizinhanca de entrada
//
// se direcao == 1, v é um vértice de um grafo direcionado e a função
//                  devolve sua vizinhanca de saída
 
lista vizinhanca(vertice v, int direcao, grafo g){
    if (!g)
        return NULL;
   
    if(direcao==-1)
        return vizinhanca_entrada(v);
    else if (direcao == 1 || direcao == 0)
        return vizinhanca_saida(v);
       
    return NULL;
}
 
//------------------------------------------------------------------------------
// devolve o grau do vértice v
// i
// se direcao == 0, v é um vértice de um grafo não direcionado
//                  e a função devolve seu grau
//
// se direcao == -1, v é um vértice de um grafo direcionado
//                   e a função devolve seu grau de entrada
//
// se direcao == 1, v é um vértice de um grafo direcionado
//                  e a função devolve seu grau de saída
 
unsigned int grau(vertice v, int direcao, grafo g){
    if (!g)
        return 0;
    if(direcao < 0)
        return v->grau_entrada;
    else
        return v->grau_saida;
}

//------------------------------------------------------------------------------
//Encontra o vertice que foi removido do grafo

static unsigned int findRemoved(lista l){
    unsigned int removed = 0 ;
    for (no n=primeiro_no(l); n!=NULL; n=proximo_no(n)) {
        vertice v = conteudo(n);
        if(v->removido == 1){
           removed ++;
        }
    }
    return removed;
}
 
//------------------------------------------------------------------------------
// devolve 1, se o conjunto dos vertices em l é uma clique em g, ou
//         0, caso contrário
//
// um conjunto C de vértices de um grafo é uma clique em g
// se todo vértice em C é vizinho de todos os outros vértices de C em g
 
    // faz um loop em cima de cada elemento da lista l      
    // para cada elemento pega sua vizinhaca
    // se a vizinhaca conter todos os elementos da lista l continua
    // se não conter retorna 0
   
int clique(lista l, grafo g){
    unsigned int removed = findRemoved(l);
    for (no n=primeiro_no(l); n!=NULL; n=proximo_no(n)) {
        vertice v = conteudo(n);
        if(v->removido == 0){  
            lista vizinhos = vizinhanca(v,0,g);
            unsigned int todos_nos = tamanho_lista(l) - removed - 1;
            for (no auxN=primeiro_no(vizinhos); auxN!=NULL; auxN=proximo_no(auxN)) {
                if(todos_nos == 0)
                    break;
                vertice auxV = conteudo(auxN);
                if(auxV->removido == 0){
                    for (no verifiyNode=primeiro_no(l); verifiyNode!=NULL; verifiyNode=proximo_no(verifiyNode)) {
                        vertice verifyVertice = conteudo(verifiyNode);  
                        if(verifyVertice->removido == 0){
                            if(strcmp(verifyVertice->nome,auxV->nome) == 0){
                                todos_nos--;
                                break;
                            }  
                        }
                    }
                }
            }
            if(todos_nos != 0){
                return 0;
            }
        }
    }  
    return 1;
}
 
//------------------------------------------------------------------------------
// devolve 1, se v é um vértice simplicial em g, ou
//         0, caso contrário
//
// um vértice é simplicial no grafo se sua vizinhança é uma clique
 
int simplicial(vertice v, grafo g){      
    lista vizinhos = vizinhanca(v,0,g);    
    return clique(vizinhos,g);
}
//------------------------------------------------------------------------------
// Gera o rotulo para o vertice v
static void generateNumberRotule(vertice v, unsigned int rotulo){
    
    v->rotulo = realloc (v->rotulo,sizeof(unsigned int)* (unsigned int)(v->tamRotulo+1));
    v->rotulo[v->tamRotulo] = rotulo;
    v->tamRotulo+=1;

}
//------------------------------------------------------------------------------
// Retorna uma lista de maiores lexicos, se nao houver retorna uma lista vazia

static lista findMaior(int posicao, unsigned int maiorRotulo, grafo g){
    lista lexicos = constroi_lista();

    for(unsigned int i=0; i < g->n_vertices; i++){
        if(g->vertices[i]->passado!=1 && g->vertices[i]->rotulo!=NULL){
            if(posicao < g->vertices[i]->tamRotulo){

                if(g->vertices[i]->rotulo[posicao] > maiorRotulo){
                    destroi_lista(lexicos, NULL);
                    lexicos = constroi_lista();
                    insere_lista(g->vertices[i],lexicos);
                    maiorRotulo = g->vertices[i]->rotulo[posicao];
                }
                else if(g->vertices[i]->rotulo[posicao] == maiorRotulo){
                    insere_lista(g->vertices[i],lexicos);  
                }
            }
        }
    }

    return lexicos;
    free(lexicos);
}
//------------------------------------------------------------------------------
// Retorna o vertice lexico

static vertice findLexico(grafo g){
        unsigned int maior = 0;
        int notFinished = 1;
        int k = 0;
        lista result;

        vertice v = NULL;

        while(notFinished){
            result = findMaior(k,maior,g);
            unsigned int tamResult = (unsigned int) tamanho_lista(result);
            if(tamResult == 0){
                notFinished = 0;            
            }
            else{
                no verticeNo=primeiro_no(result);
                v = conteudo(verticeNo);
            }
            k++;
        }

        return v;
	free(result);
}
//------------------------------------------------------------------------------
// Rotula a vizinhaca do vertice raiz passado por parametro.
static void rotulaVizinhaca(vertice raiz, grafo g){
    lista vizinhos = vizinhanca(raiz,0,g);    
    for (no n=primeiro_no(vizinhos); n!=NULL; n=proximo_no(n)) {
        unsigned int rotuloRaiz = raiz->rotulo[0];
        
        vertice auxV = conteudo(n);
        generateNumberRotule(auxV,rotuloRaiz-1);
    }  
    free(vizinhos);
}

//------------------------------------------------------------------------------
// devolve uma lista de vertices com a ordem dos vértices dada por uma 
// busca em largura lexicográfica

lista busca_largura_lexicografica(grafo g){
    lista arvore = constroi_lista();
    int naoTerminou=1;
    if(g->vertices!=NULL){
        unsigned int rotulo = g->n_vertices;
        vertice raiz = g->vertices[0];
        generateNumberRotule(raiz,rotulo);
        rotulaVizinhaca(raiz,g);
        raiz->passado = 1;
        insere_lista(raiz, arvore);
        while(naoTerminou){
            vertice lexico = findLexico(g);
            if(lexico!=NULL){
                rotulaVizinhaca(lexico,g);
                insere_lista(lexico, arvore);
                lexico->passado = 1;
            }
            else{
                naoTerminou = 0;
            }

        }
    }

    return arvore;
    free(arvore);
}
//------------------------------------------------------------------------------
// Procura o vertice v da vizinhaca de AuxN na lista léxica e retorna a posição dele em relação ao auxN.
static int leftPosition (vertice w, no auxN){
    int index = 0;
    for(no auxViz=proximo_no(auxN); auxViz!=NULL; auxViz=proximo_no(auxViz)){
        vertice v = conteudo(auxViz);
        if(strcmp(w->nome,v->nome) == 0){
            return index;
        }
        index++;
    }
    return -1;
}

//------------------------------------------------------------------------------
// devolve 1, se a lista l representa uma 
//            ordem perfeita de eliminação para o grafo g ou
//         0, caso contrário
//
// o tempo de execução é O(|V(G)|+|E(G)|)

int ordem_perfeita_eliminacao(lista l, grafo g){
	
    for (no auxN=primeiro_no(l); auxN!=NULL; auxN=proximo_no(auxN)) {
        int menor = -1;
        vertice v = conteudo(auxN);
        
        vertice w = NULL;
        lista vizinhos = vizinhanca(v,0,g);
        for(no auxViz=primeiro_no(vizinhos); auxViz!=NULL; auxViz=proximo_no(auxViz)){
            vertice auxRight = conteudo(auxViz);
            if(auxRight->removido==0){
               int index = leftPosition(auxRight, auxN);
               if(((menor==-1 ) || (index < menor)) && index>=0){
                    menor = index;
                    w = auxRight;
               }
            }
        }

        if(w!=NULL){
            lista vizinhosW = vizinhanca(w,0,g);
            int notFound;
            for(no auxVizV=primeiro_no(vizinhos); auxVizV!=NULL; auxVizV=proximo_no(auxVizV)){
                vertice auxV = conteudo(auxVizV);
                if(strcmp(auxV->nome,w->nome) != 0 && auxV->removido == 0){
                    
                    notFound = 1;
                    for(no auxVizW=primeiro_no(vizinhosW); auxVizW!=NULL; auxVizW=proximo_no(auxVizW)){
                       vertice auxW = conteudo(auxVizW);
                       
                       if(strcmp(auxW->nome,auxV->nome) == 0){
                           notFound = 0;
                           break;
                        }
                    }
                    if(notFound)
                        return 0;
                }
            }
            v->removido=1;
        }
    }
    return 1;
}
//------------------------------------------------------------------------------
// devolve 1, se g é um grafo cordal ou
//         0, caso contrário

int cordal(grafo g){
    grafo copy = copia_grafo(g);
    lista lexica = busca_largura_lexicografica(copy);
    return ordem_perfeita_eliminacao(lexica,copy);
    free(copy);
    free(lexica);
}



static int adicionaConjunto(lista conjOrigem, lista conjDestino, grafo g){
    int terminado = 1;
    for(no pai=primeiro_no(conjOrigem); pai!=NULL; pai=proximo_no(pai)){
        vertice auxV = conteudo(pai);
        printf("Pai : %s\n",auxV->nome);
        if(auxV->passado == 0){
            terminado = 0;
            auxV->passado = 1;
            lista filhos = vizinhanca(auxV,0,g);
            for(no filho=primeiro_no(filhos); filho!=NULL; filho=proximo_no(filho)){
                vertice auxVa = conteudo(filho);
                if(auxVa->inSet != 1){
                    printf("Filho: %s\n",auxVa->nome);
                    insere_lista(auxVa,conjDestino);        
                    auxVa->inSet = 1;
                }
            }       
        }
    }
    return terminado;
}

static void bipartido(lista conjA, lista conjB, grafo g){
    int terminado = 0;
    vertice auxV = NULL;
    vertice *vertices = g->vertices;
    vertice v = vertices[0];        
    insere_lista(v,conjA);
    lista filhos = vizinhanca(v,0,g);
    for(no auxVizV=primeiro_no(filhos); auxVizV!=NULL; auxVizV=proximo_no(auxVizV)){
        auxV = conteudo(auxVizV);
        insere_lista(auxV,conjB);   
        auxV->inSet = 1;     
    }
    v->inSet = 1;
    v->passado = 1;
    while(!terminado){
        terminado = 1;
        terminado = adicionaConjunto(conjB, conjA, g);
        terminado = adicionaConjunto(conjA, conjB, g);
    }
}

static void firstMatching(lista conjA, lista edgeMatching, grafo g){

    for(no auxVizV=primeiro_no(conjA); auxVizV!=NULL; auxVizV=proximo_no(auxVizV)){
        vertice v = conteudo(auxVizV);
        lista filhos = vizinhanca(v,0,g);
        for(no filhoNo=primeiro_no(filhos); filhoNo!=NULL; filhoNo=proximo_no(filhoNo)){
            vertice filho = conteudo(filhoNo);
            if(filho->coberto!=1){
                filho->coberto = 1;
                v->coberto = 1;
                adjacencia viz = malloc(sizeof(struct adjacencia));
                viz->v_origem = v;
                viz->v_destino = filho;
                insere_lista(viz,edgeMatching);
                break;
            }
        }
    }  
        
}



//------------------------------------------------------------------------------
// devolve um grafo cujos vertices são cópias de vértices do grafo
// bipartido g e cujas arestas formam um emparelhamento máximo em g
//
// não verifica se g é bipartido; caso não seja, o comportamento é indefinido

// LINK do algoritmo: https://en.wikipedia.org/wiki/Hopcroft%E2%80%93Karp_algorithm#Algorithm

grafo emparelhamento_maximo(grafo g){
    lista conjA = constroi_lista();
    lista conjB = constroi_lista();
    bipartido(conjA,conjB,g);
    printf("Conjunto A:\n");
    for(no auxVizVa=primeiro_no(conjA); auxVizVa!=NULL; auxVizVa=proximo_no(auxVizVa)){
        vertice auxVa = conteudo(auxVizVa);
        printf("%s\n",auxVa->nome);
    }

    printf("\n\nConjunto B:\n");
    for(no auxVizVb=primeiro_no(conjB); auxVizVb!=NULL; auxVizVb=proximo_no(auxVizVb)){
        vertice auxVb = conteudo(auxVizVb);
        printf("%s\n",auxVb->nome);
    }

    lista edgeMatching = constroi_lista();

    

    firstMatching(conjA, edgeMatching, g);

    printf("\n\nLigacoes:\n");
    for(no edgeNo=primeiro_no(edgeMatching); edgeNo!=NULL; edgeNo=proximo_no(edgeNo)){
        adjacencia a= conteudo(edgeNo);
        printf("Origem: %s\n",a->v_origem->nome);
        printf("Destino: %s\n",a->v_destino->nome);
    }

    return g;
}

