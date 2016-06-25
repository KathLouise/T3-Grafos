 +------------------------------------------------------------------+
 | Katheryne Louise Graf - GRR20120706 - Indentificador: klg12		|
 | Rafael Rocha de Carvalho - GRR20104490 - Indentificador: rrc10   |
 | 2º Trabalho de CI065 - 2016/1									|
 +------------------------------------------------------------------+

Arquivos:

	- grafo.c: Implementação das estruturas do grafo
	- lista.c: Implementação das estruturas da lista (fornecida pelo professor)

Estrutura de Dados:
	No trabalho foram implementadas três estruturas: grafo, vertice e aresta.
	Sobre elas:
	- A estrutura do grafo possui o nome do grafo, uma variavel para dizer se ele é direcionado ou não, uma variavel para dizer se ele é ponderado ou não, numero total de vertices, numero total de arestas e um vetor de vertices.
	
	- A estrutura do vertice possui nome, sua posição no vetor, uma flag de estado removido, um inteiro que guarda o tamanho do vetor de rotulos, uma flag de estado passado, uma flag de visitado, uma flag de coberto, uma flag de inSet, uma flag de set, um vetor de inteiro para guardar os rotulos, duas listas de adjacencias (entrada e saida), e seus graus de entrada e de saida.
	
	- A estrutura de adjacencia possui peso, vertice de origem e vertice de destino.
	
	- A representação do grafo foi feita por um vetor de vértices, onde cada vértice contém uma lista de suas adjacências.

	- Cada vértice também armazena qual é o seu índice no vetor de vértices do grafo (vertice->id), facilitando algumas operações como copiar uma aresta/arco de um grafo para outro (somente quando ambos os grafos possuem o mesmo conjunto de vértices).  

	- Dentro de cada vértice, existem 2 listas de adjacências, sendo uma lista de adjacências de entrada, e a outra de adjacências de saída; está decisão foi tomada como medida para, principalmente, tornar mais fácil a implementação da função de obtenção das vizinhanças, e melhorar sua eficiência para quando se trata de grafos direcionados.
	
	- Foi escolhido adotar o uso de uma flag de removido para evitar problemas com alocação e desalocação de memoria no momento de verificar se o vertice é simplicial dentro da função cordal, assim evitando problemas com vazamento de memoria, principalmente.
	
	- Para a rotulagem dos vertices do grafo, optou-se por usar um vetor, que é realocado a cada vez que um vertice ganha um novo rotulo com o seu tamanho aumentado em uma posição.

Implementação:

	Alem das funções solicitadas pelo professor, foram implementadas mais algumas para facilitar a manipulação do grafo.

	- static grafo cria_grafo

		Cria e devolve um  grafo g

	- static void cria_vizinhanca

		Cria um vizinho/aresta e insere na lista de vizinhos do vértice de origem e/ou na de destino também se ele não for direcionado

	- static int destroi_vizinho
	
		Destrói um vizinho/aresta

	- static vertice cria_vertice

		Cria, insere no grafo e retorna o vértice.

	- static void destroi_vertice

		Destrói um vértice

	- static vertice v_busca

		Busca um vértice pelo nome do grafo

 
	- static long int get_peso

		Devolve o peso de uma aresta no formato libcgraph

	- static int contem_pesos(Agraph_t *Ag) 

		Devolve 1 se o grafo no formato libcgraph tem pesos nas arestas

	- static lista vizinhanca_entrada(vertice v)
		
		Devolve a vizinhança de entrada do vértice v

	- static lista vizinhanca_saida

		Devolve a vizinhança de saída do vértice v
	
	- static unsigned int findRemoved
		
		Encontra o vertice que foi removido do grafo
		
	- static void generateNumberRotule
		Gera o rotulo para o vertice v
	
	- static lista findMaior
		Retorna uma lista de maiores lexicos, se nao houver retorna uma lista vazia
	
	- static vertice findLexico
		Retorna o vertice lexico
	
	- static void rotulaVizinhaca
		Rotula a vizinhaca do vertice raiz passado por parametro.
	
	- static int leftPosition
		Procura o vertice v da vizinhaca de AuxN na lista léxica e retorna a posição dele em relação ao auxN.

	- static void firstMatching
		Encontra um primeiro emparelhamento no grafo

	- static int adicionaConjunto
		Adiciona os vertices a uma das duas partições

	- static void bipartido
		Separa o grafo em duas partições

	- static void settingConj
		Adiciona o id do conjunto em que o vertice esta inserido

	- static vertice findInMatching
		Procura um vertice de origem dentro da lista do primeiro emparelhamento
		Se encontrar, retorna este vertice
		Caso contrário, retorna NULL

	- static void resetVisitado
		reseta para 0 a flag passado, pois ela será reutilizada

	- static lista findAumentingPath
		Se houver um caminho de um vertice descoberto, procura um caminho que o ligue
		até outro vertice do conjunto oposto que também esta descoberto
		retorna o caminho, se tiver
		retorna NULL, caso contrário

	- static void changeMatching
		Executar o xor entre a lista que contem o caminho aumentante e o primeiro 
		emparelhamento encontrado

	- static void resetFlag
		reseta para 0 a flag passado, pois ela será reutilizada

	- static int allCovered
		Faz uma busca para verificar se todos os vertices da partição estão cobertos
		retorna 1, se estiverem
		retorna 0, caso contrario.

Bugs Conhecidos:
	Foram feitos vários testes com vários tamanhos de grafos, cordais, não cordais, direcionados, não direcionados, ponderados e não ponderados. Nestes testes, os bugs encontrados foram resolvidos, então neste momento não há nenhum bugs conhecidos nesta presente versão.
