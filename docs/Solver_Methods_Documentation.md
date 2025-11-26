# 📘 Documentação dos Métodos da Classe Solver

**Arquivo**: `src/Solver.cpp`
**Autor**: Isaac Kosloski
**Data de Criação**: 03/02/2025
**Última Atualização**: Janeiro 2025

---

## 📋 Índice

1. [Construtor](#1-construtor)
2. [Heurísticas Construtivas](#2-heurísticas-construtivas)
   - [initialSolution_Greedy](#21-initialsolution_greedy)
   - [GRASP_Construct](#22-grasp_construct)
3. [Heurísticas de Busca Local](#3-heurísticas-de-busca-local)
   - [localSearch_TwoOpt](#31-localsearch_twoopt)
   - [localSearch_ThreeOpt](#32-localsearch_threeopt)
   - [localSearch_SwapStar](#33-localsearch_swapstar)
4. [Funções Auxiliares](#4-funções-auxiliares)
   - [calculate_swap_star_delta](#41-calculate_swap_star_delta)
5. [Critério de Aceitação](#5-critério-de-aceitação)
   - [acceptanceCriterion_BestSolution](#51-acceptancecriterion_bestsolution)

---

## 1. Construtor

### `Solver::Solver()`

**Linha**: 5-8

**Descrição de Alto Nível**:
Construtor padrão da classe Solver. Inicializa uma instância do resolvedor sem parâmetros. Atualmente vazio, pois a inicialização de estruturas é feita através dos métodos de solução.

**Parâmetros**: Nenhum

**Retorno**: Objeto Solver inicializado

**Complexidade**: O(1)

**Uso Típico**:
```cpp
auto *solver = new Solver();
```

---

## 2. Heurísticas Construtivas

### 2.1. `initialSolution_Greedy`

**Linha**: 10-71

**Assinatura**:
```cpp
void initialSolution_Greedy(CVRP *instance, Solution *initialSolution)
```

**Descrição de Alto Nível**:
Gera uma solução inicial usando uma heurística gulosa pura (greedy). O algoritmo constrói rotas sequencialmente, sempre selecionando o cliente não visitado mais próximo do cliente atual que respeite a restrição de capacidade do veículo. Quando nenhum cliente pode ser adicionado à rota atual (por limitação de capacidade), uma nova rota é iniciada.

**Estratégia**:
1. Inicia no depósito com o primeiro veículo
2. Para cada iteração:
   - Encontra o cliente não visitado mais próximo que caiba no veículo
   - Se encontrado: adiciona à rota atual
   - Se não encontrado: fecha a rota atual, retorna ao depósito e inicia nova rota
3. Continua até que todos os clientes sejam visitados
4. Calcula o custo total da solução

**Parâmetros**:
- `instance`: Ponteiro para a instância do problema CVRP (contém nós, distâncias, capacidades)
- `initialSolution`: Ponteiro para o objeto Solution que será preenchido com a solução gerada

**Características**:
- **Determinístico**: Sempre produz a mesma solução para a mesma instância
- **Guloso**: Escolhe sempre a opção localmente ótima (menor distância)
- **Rápido**: Execução eficiente, ideal como solução inicial
- **Qualidade**: Boa qualidade, mas pode ficar preso em ótimos locais

**Complexidade**: O(n²) onde n é o número de clientes

**Estruturas Modificadas**:
- `initialSolution->routes`: Lista de rotas geradas
- `initialSolution->routeLoads`: Cargas de cada rota
- `initialSolution->totalCost`: Custo total da solução
- `initialSolution->fleetSize`: Número de veículos utilizados

---

### 2.2. `GRASP_Construct`

**Linha**: 73-149

**Assinatura**:
```cpp
void GRASP_Construct(CVRP *instance, Solution *initialSolution, double alpha)
```

**Descrição de Alto Nível**:
Implementa a fase construtiva do algoritmo GRASP (Greedy Randomized Adaptive Search Procedure). Combina elementos gulosos e aleatórios para gerar soluções diversificadas. O algoritmo constrói uma Lista Restrita de Candidatos (RCL) baseada no parâmetro α e seleciona aleatoriamente um candidato desta lista em cada iteração.

**Estratégia - Restricted Candidate List (RCL)**:
1. Para cada posição a ser preenchida:
   - Identifica todos os candidatos viáveis (respeitam capacidade)
   - Calcula custo de inserção para cada candidato
   - Ordena candidatos por custo crescente
2. Constrói RCL usando o parâmetro α:
   - `threshold = minCost + α × (maxCost - minCost)`
   - Inclui na RCL todos os candidatos com custo ≤ threshold
3. Seleciona aleatoriamente um candidato da RCL
4. Adiciona o candidato à rota e atualiza estruturas
5. Repete até completar todas as rotas

**Parâmetros**:
- `instance`: Ponteiro para a instância do problema CVRP
- `initialSolution`: Ponteiro para o objeto Solution que receberá a solução construída
- `alpha`: Parâmetro de aleatoriedade (0 ≤ α ≤ 1)
  - α = 0: Comportamento totalmente guloso (equivale ao Greedy)
  - α = 1: Seleção completamente aleatória entre todos os candidatos
  - α ∈ (0,1): Balanceamento entre guloso e aleatório (recomendado: 0.2-0.4)

**Características**:
- **Não-determinístico**: Produz soluções diferentes a cada execução
- **Diversificação**: Explora diferentes regiões do espaço de soluções
- **Adaptativo**: α ajustável conforme a instância do problema
- **Intensificação vs Diversificação**: Controlada pelo parâmetro α

**Complexidade**: O(n² log n) onde n é o número de clientes

**Vantagens sobre Greedy Puro**:
- Gera diversidade de soluções
- Evita ficar preso em ótimos locais
- Base fundamental do metaheurístico GRASP

---

## 3. Heurísticas de Busca Local

### 3.1. `localSearch_TwoOpt`

**Linha**: 152-200

**Assinatura**:
```cpp
void localSearch_TwoOpt(CVRP *instance, Solution *initialSolution, Solution *bestSolution)
```

**Descrição de Alto Nível**:
Implementa a heurística de busca local 2-Opt, uma das técnicas mais eficazes para refinamento de rotas em problemas de roteamento. O algoritmo examina pares de arestas em cada rota e testa se trocar estas arestas (invertendo o segmento entre elas) resulta em uma rota com menor custo.

**Estratégia - Operador 2-Opt**:
```
Rota Original:  depot → A → B → C → D → depot
                         ↓       ↓
Segmento:               [B → C]
                         ↓       ↓
Após 2-Opt:     depot → A → C → B → D → depot
                       (segmento invertido)
```

**Funcionamento Detalhado**:
1. Para cada rota na solução:
   - Examina todos os pares de arestas (i, j) onde i < j
   - Para cada par:
     - **Aresta 1**: A → B (antes do segmento)
     - **Aresta 2**: C → D (depois do segmento)
   - Calcula custo atual: dist(A,B) + dist(C,D)
   - Calcula novo custo: dist(A,C) + dist(B,D)
   - Se novo custo < custo atual:
     - Reverte o segmento [B...C]
     - Atualiza o custo total da solução
     - Marca que houve melhoria
2. Repete o processo enquanto houver melhorias (até convergir)

**Parâmetros**:
- `instance`: Ponteiro para a instância do problema
- `initialSolution`: Solução de entrada a ser refinada
- `bestSolution`: Solução de saída após o refinamento

**Características**:
- **Intra-rota**: Opera dentro de cada rota individualmente
- **Intensificação**: Melhora a qualidade da solução localmente
- **Iterativo**: Continua até não encontrar mais melhorias
- **Limite de Iterações**: Máximo de 1000 iterações para evitar loops infinitos

**Complexidade**: O(n² × k) onde:
- n = tamanho médio das rotas
- k = número de iterações até convergência

**Condição de Parada**:
- Nenhuma melhoria encontrada (ótimo local atingido)
- OU limite de 1000 iterações atingido

**Melhoria Típica**: 5-15% sobre a solução inicial

---

### 3.2. `localSearch_ThreeOpt`

**Linha**: 202-265

**Assinatura**:
```cpp
void localSearch_ThreeOpt(CVRP *instance, Solution *initialSolution, Solution *bestSolution)
```

**Descrição de Alto Nível**:
Implementa a heurística 3-Opt, uma generalização mais poderosa do 2-Opt. Enquanto o 2-Opt remove duas arestas, o 3-Opt remove três arestas simultaneamente e reconecta os segmentos de diferentes formas, explorando um espaço de busca maior e potencialmente encontrando melhores soluções.

**Estratégia - Operador 3-Opt**:
```
Rota Original:  depot → A → B → C → D → E → depot
                         ↓   ↓   ↓
Três pontos:            i   j   k
                         ↓   ↓   ↓
Segmentos:          [A-B] [B-C] [C-D]
```

**Cinco Possíveis Reconfigurações**:
1. **Original**: Mantém a rota como está
2. **Reverter(i,j)**: Inverte segmento entre i e j
3. **Reverter(j,k)**: Inverte segmento entre j e k
4. **Reverter(i,j) + Reverter(j,k)**: Inverte ambos os segmentos
5. **Reverter(i,k)**: Inverte todo o segmento de i a k

**Funcionamento Detalhado**:
1. Para cada rota:
   - Requer no mínimo 5 nós: [depot, a, b, c, depot]
2. Para cada tripla (i, j, k) onde i < j < k:
   - Gera 5 configurações alternativas
   - Calcula o custo de cada configuração
   - Se alguma configuração tem custo menor:
     - Adota a nova configuração
     - Atualiza bestSolution
     - Marca melhoria
3. Continua até não haver melhorias

**Parâmetros**:
- `instance`: Ponteiro para a instância do problema
- `initialSolution`: Solução de entrada
- `bestSolution`: Melhor solução encontrada (saída)

**Características**:
- **Mais Poderoso que 2-Opt**: Explora espaço de busca maior
- **Computacionalmente Mais Caro**: O(n³) vs O(n²) do 2-Opt
- **Melhor Qualidade**: Pode escapar de ótimos locais onde 2-Opt fica preso
- **Trade-off**: Tempo vs Qualidade

**Complexidade**: O(n³ × k) onde:
- n = tamanho médio das rotas
- k = número de iterações até convergência

**Quando Usar**:
- Instâncias pequenas a médias (< 100 clientes)
- Quando tempo não é crítico
- Para refinar soluções após 2-Opt

**Melhoria Adicional sobre 2-Opt**: 2-8%

---

### 3.3. `localSearch_SwapStar`

**Linha**: 267-366

**Assinatura**:
```cpp
void localSearch_SwapStar(CVRP *instance, Solution *solution, int chain_length)
```

**Descrição de Alto Nível**:
Implementa a heurística Swap-Star (também conhecida como Chain Relocation), que move sequências (cadeias) de clientes de uma posição para outra, seja dentro da mesma rota (intra-rota) ou entre rotas diferentes (inter-rotas). É particularmente eficaz para balancear cargas entre veículos e melhorar a distribuição espacial dos clientes.

**Estratégia - Operador Swap-Star**:
```
Movimento Intra-Rota:
Antes:  depot → A → [B → C → D] → E → F → depot
                    └─────┬─────┘
Depois: depot → A → E → [B → C → D] → F → depot
                        └─────┬─────┘

Movimento Inter-Rotas:
Rota 1: depot → A → [B → C] → D → depot
                    └───┬───┘
                        ↓ movido para ↓
Rota 2: depot → E → F → [B → C] → G → depot
```

**Funcionamento Detalhado**:
1. **Sincronização de Cargas**:
   - Garante que `routeLoads` está sincronizado com `routes`
   - Calcula a demanda total de cada rota

2. **Busca de Movimentos**:
   - Para cada par de rotas (r1, r2):
     - Para cada posição i em r1 (origem da cadeia):
       - Extrai cadeia de comprimento `chain_length`
       - Calcula demanda da cadeia
       - **Verifica Capacidade**: Se movimento inter-rotas, checa se rota destino suporta a carga adicional
       - Para cada posição j em r2 (destino):
         - Calcula delta de custo do movimento
         - Se delta < 0 (melhoria):
           - Executa o movimento
           - Atualiza cargas das rotas
           - Reinicia busca (first improvement)

3. **Cálculo de Delta**:
   - **Intra-rota**: Custo complexo dependendo de j < i ou j > i
   - **Inter-rotas**: Remove arestas antigas, adiciona novas arestas

**Parâmetros**:
- `instance`: Ponteiro para a instância do problema
- `solution`: Solução a ser refinada (modificada in-place)
- `chain_length`: Comprimento da cadeia a ser movida (tipicamente 1-3)

**Características**:
- **Intra-rota e Inter-rotas**: Opera nos dois níveis
- **First Improvement**: Ao encontrar melhoria, aplica imediatamente e reinicia
- **Verificação de Capacidade**: Garante viabilidade da solução
- **Epsilon Tolerance**: Usa ε = 1e-5 para comparações de ponto flutuante

**Complexidade**: O(n² × m²) onde:
- n = número médio de clientes por rota
- m = número de rotas

**Parâmetros Recomendados**:
- `chain_length = 1`: Movimento de um único cliente (mais rápido)
- `chain_length = 2`: Movimento de pares (melhor qualidade)
- `chain_length = 3`: Movimento de triplas (mais lento, melhor qualidade)

**Vantagens**:
- Balanceia cargas entre veículos
- Melhora distribuição espacial
- Pode reduzir número de veículos necessários

---

## 4. Funções Auxiliares

### 4.1. `calculate_swap_star_delta`

**Linha**: 368-417

**Assinatura**:
```cpp
double calculate_swap_star_delta(CVRP *instance, const vector<int>& route1,
                                 const vector<int>& route2, int i, int j, int k)
```

**Descrição de Alto Nível**:
Calcula a variação de custo (delta) resultante de um movimento Swap-Star sem executá-lo de fato. Esta função é crítica para a eficiência do Swap-Star, pois permite avaliar centenas de movimentos rapidamente sem modificar a solução.

**Estratégia de Cálculo**:

**Caso 1: Movimento Inter-Rotas** (route1 ≠ route2)
```
Rota 1: ... → A → [B...C] → D → ...
              ↓    cadeia   ↓
Remove: dist(A,B) + dist(C,D)

Rota 2: ... → E → F → ...
                ↓
Remove: dist(E,F)

Após movimento:
Rota 1: ... → A → D → ...
Rota 2: ... → E → [B...C] → F → ...

Adiciona: dist(A,D) + dist(E,B) + dist(C,F)
```

**Caso 2: Movimento Intra-Rota** (route1 == route2)
- **Subcaso j < i**: Cadeia move para antes de sua posição original
- **Subcaso j > i**: Cadeia move para depois de sua posição original

**Parâmetros**:
- `instance`: Ponteiro para a instância (acesso à matriz de distâncias)
- `route1`: Rota de origem (passada por referência constante)
- `route2`: Rota de destino (passada por referência constante)
- `i`: Índice inicial da cadeia na rota de origem
- `j`: Índice de inserção na rota de destino
- `k`: Comprimento da cadeia (`chain_length`)

**Retorno**:
- Valor double representando Δcusto = custo_novo - custo_antigo
- **Δ < 0**: Movimento melhora a solução
- **Δ > 0**: Movimento piora a solução
- **Δ ≈ 0**: Movimento neutro

**Nós Envolvidos no Cálculo**:
- **A**: Nó antes da cadeia em route1
- **B**: Primeiro nó da cadeia
- **C**: Último nó da cadeia
- **D**: Nó depois da cadeia em route1
- **E**: Nó antes do ponto de inserção em route2
- **F**: Nó no ponto de inserção em route2

**Validação**:
- Verifica bounds dos índices antes de acessar
- Retorna 0.0 (delta neutro) se índices inválidos
- Evita segmentation faults

**Complexidade**: O(1) - acesso direto à matriz de distâncias

**Importância**:
- **Performance**: Avalia movimentos em O(1) ao invés de O(n)
- **Eficiência**: Permite testar milhares de movimentos rapidamente
- **Precisão**: Cálculo exato do impacto no custo

---

## 5. Critério de Aceitação

### 5.1. `acceptanceCriterion_BestSolution`

**Linha**: 420-429

**Assinatura**:
```cpp
void acceptanceCriterion_BestSolution(Solution *bestSolution, Solution *newSolution)
```

**Descrição de Alto Nível**:
Implementa o critério de aceitação elitista para o algoritmo GRASP. Compara a nova solução gerada com a melhor solução conhecida até o momento e atualiza a melhor solução apenas se a nova for estritamente melhor (menor custo).

**Estratégia - Elitismo Puro**:
```
if (newSolution.cost < bestSolution.cost):
    bestSolution = newSolution  // Aceita
else:
    // Rejeita (mantém bestSolution)
```

**Funcionamento**:
1. Compara `newSolution->totalCost` com `bestSolution->totalCost`
2. Se nova solução tem custo menor:
   - Copia completamente a nova solução para bestSolution
   - Atualiza: rotas, cargas, custo total, tamanho da frota
3. Se nova solução não é melhor:
   - Descarta a nova solução
   - Mantém a melhor solução atual

**Parâmetros**:
- `bestSolution`: Ponteiro para a melhor solução encontrada até agora (entrada/saída)
- `newSolution`: Ponteiro para a nova solução candidata (entrada)

**Características**:
- **Elitismo**: Nunca piora a qualidade da solução
- **Determinístico**: Sempre aceita se e somente se houver melhoria
- **Monotonia**: O custo de bestSolution é não-crescente ao longo das iterações
- **Simples**: Implementação direta e eficiente

**Complexidade**: O(n + m) onde:
- n = número total de clientes
- m = número de rotas
- (devido à cópia profunda da estrutura Solution)

**Alternativas Não Implementadas**:
- **Simulated Annealing**: Aceita soluções piores com probabilidade decrescente
- **Threshold Accepting**: Aceita soluções até um threshold
- **Tabu Search**: Usa memória de movimentos proibidos
- **Relaxed Elite**: Aceita soluções dentro de um percentual da melhor

**Papel no GRASP**:
- Mantém a melhor solução global ao longo de múltiplas iterações
- Garante que o algoritmo sempre retorna uma solução de qualidade
- Permite que GRASP explore diferentes regiões (via construção randomizada) sem perder a melhor solução

**Output (Comentado)**:
```cpp
//cout << "New best solution found with cost: " << bestSolution->totalCost << endl;
```
- Linha comentada para evitar poluir o console
- Útil para debugging e acompanhamento da convergência

---

## 📊 Resumo Comparativo das Heurísticas

| Método | Tipo | Complexidade | Qualidade | Uso Típico |
|--------|------|-------------|-----------|------------|
| **initialSolution_Greedy** | Construtiva | O(n²) | Boa | Solução inicial rápida |
| **GRASP_Construct** | Construtiva | O(n² log n) | Boa-Excelente | Base do metaheurístico |
| **localSearch_TwoOpt** | Busca Local | O(n² × k) | Muito Boa | Refinamento padrão |
| **localSearch_ThreeOpt** | Busca Local | O(n³ × k) | Excelente | Instâncias pequenas |
| **localSearch_SwapStar** | Busca Local | O(n² × m²) | Excelente | Balanceamento de rotas |

---

## 🔄 Fluxo Típico no GRASP

```
1. Greedy Inicial → Solução base (1x)
                    ↓
2. Loop (1000 iterações):
   ├─ GRASP_Construct (α=0.3) → Solução diversificada
   ├─ TwoOpt → Refinamento local
   └─ AcceptanceCriterion → Atualiza melhor se necessário
                    ↓
3. Retorna: Melhor solução encontrada
```

---

## 📝 Notas Importantes

### Gerenciamento de Memória
- Todos os métodos recebem ponteiros para estruturas já alocadas
- Nenhum método aloca ou desaloca memória dinamicamente
- Responsabilidade de limpeza está no `main.cpp`

### Determinismo vs. Aleatoriedade
- **Determinísticos**: `initialSolution_Greedy`, `localSearch_*`, `acceptanceCriterion_*`
- **Não-determinístico**: `GRASP_Construct` (usa `rand()`)

### Seed Aleatória
- Para reprodutibilidade, defina seed antes de chamar GRASP_Construct:
  ```cpp
  srand(42); // Resultados reproduzíveis
  ```

### Performance
- **Mais Rápido**: Greedy + TwoOpt
- **Melhor Qualidade**: GRASP + TwoOpt (múltiplas iterações)
- **Melhor Balanceamento**: Adicionar SwapStar após TwoOpt

### Limite de Iterações
- Todos os métodos de busca local têm `maxIterations = 1000`
- Previne loops infinitos em casos patológicos
- Pode ser ajustado conforme necessário

---

## 🎯 Recomendações de Uso

### Para Instâncias Pequenas (< 50 clientes)
```cpp
solver->initialSolution_Greedy(cvrp, solution);
solver->localSearch_ThreeOpt(cvrp, solution, bestSolution);
solver->localSearch_SwapStar(cvrp, bestSolution, 2);
```

### Para Instâncias Médias (50-200 clientes)
```cpp
for(int i = 0; i < 1000; i++) {
    solver->GRASP_Construct(cvrp, solution, 0.3);
    solver->localSearch_TwoOpt(cvrp, solution, tempSolution);
    solver->acceptanceCriterion_BestSolution(bestSolution, tempSolution);
}
```

### Para Instâncias Grandes (> 200 clientes)
```cpp
// Use apenas GRASP + TwoOpt
// Evite ThreeOpt e SwapStar (muito lentos)
solver->GRASP_Construct(cvrp, solution, 0.2); // α menor para mais guloso
solver->localSearch_TwoOpt(cvrp, solution, bestSolution);
```

---

## 📚 Referências

1. **Feo & Resende (1995)**: "Greedy Randomized Adaptive Search Procedures" - Fundamentos do GRASP
2. **Croes (1958)**: "A Method for Solving Traveling-Salesman Problems" - Origem do 2-Opt
3. **Lin (1965)**: "Computer solutions of the traveling salesman problem" - 3-Opt e k-Opt
4. **Toth & Vigo (2014)**: "Vehicle Routing: Problems, Methods, and Applications" - Estado da arte em VRP

---

**Documento gerado por**: Claude Code
**Data**: Janeiro 2025
**Versão**: 1.0
