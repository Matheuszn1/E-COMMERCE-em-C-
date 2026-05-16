
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <limits>
#include <chrono>
#include <string>
using namespace std;

struct Produto {
    int codigo;
    char nome[60];
    char categoria[40];
    double preco;
    int estoque;
    double avaliacao;
    char descricao[120];
};

struct NoAVL {
    int codigo;
    int indice;
    int altura;
    NoAVL* esquerda;
    NoAVL* direita;
};

struct IndiceNome {
    string nome;
    int codigo;
    int indice;
};

struct ResultadoDesempenho {
    string operacao;
    string algoritmo;
    int resultados;
    int repeticoes;
    long long tempoTotal;
};

const string ARQUIVO = "produtos.dat";

void limparBuffer() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void copiarTexto(char destino[], int tamanho, const string& texto) {
    strncpy(destino, texto.c_str(), tamanho - 1);
    destino[tamanho - 1] = '\0';
}

string minusculo(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return tolower(c); });
    return s;
}

int altura(NoAVL* no) {
    return no ? no->altura : 0;
}

int fatorBalanceamento(NoAVL* no) {
    return no ? altura(no->esquerda) - altura(no->direita) : 0;
}

NoAVL* rotacaoDireita(NoAVL* y) {
    NoAVL* x = y->esquerda;
    NoAVL* t2 = x->direita;

    x->direita = y;
    y->esquerda = t2;

    y->altura = max(altura(y->esquerda), altura(y->direita)) + 1;
    x->altura = max(altura(x->esquerda), altura(x->direita)) + 1;

    return x;
}

NoAVL* rotacaoEsquerda(NoAVL* x) {
    NoAVL* y = x->direita;
    NoAVL* t2 = y->esquerda;

    y->esquerda = x;
    x->direita = t2;

    x->altura = max(altura(x->esquerda), altura(x->direita)) + 1;
    y->altura = max(altura(y->esquerda), altura(y->direita)) + 1;

    return y;
}

NoAVL* inserirAVL(NoAVL* raiz, int codigo, int indice) {
    if (!raiz) {
        return new NoAVL{codigo, indice, 1, nullptr, nullptr};
    }

    if (codigo < raiz->codigo) {
        raiz->esquerda = inserirAVL(raiz->esquerda, codigo, indice);
    } else if (codigo > raiz->codigo) {
        raiz->direita = inserirAVL(raiz->direita, codigo, indice);
    } else {
        raiz->indice = indice;
        return raiz;
    }

    raiz->altura = 1 + max(altura(raiz->esquerda), altura(raiz->direita));
    int fator = fatorBalanceamento(raiz);

    if (fator > 1 && codigo < raiz->esquerda->codigo) {
        return rotacaoDireita(raiz);
    }

    if (fator < -1 && codigo > raiz->direita->codigo) {
        return rotacaoEsquerda(raiz);
    }

    if (fator > 1 && codigo > raiz->esquerda->codigo) {
        raiz->esquerda = rotacaoEsquerda(raiz->esquerda);
        return rotacaoDireita(raiz);
    }

    if (fator < -1 && codigo < raiz->direita->codigo) {
        raiz->direita = rotacaoDireita(raiz->direita);
        return rotacaoEsquerda(raiz);
    }

    return raiz;
}

int buscarIndiceAVL(NoAVL* raiz, int codigo) {
    while (raiz) {
        if (codigo == raiz->codigo) return raiz->indice;
        if (codigo < raiz->codigo) raiz = raiz->esquerda;
        else raiz = raiz->direita;
    }

    return -1;
}

void liberarAVL(NoAVL* raiz) {
    if (!raiz) return;
    liberarAVL(raiz->esquerda);
    liberarAVL(raiz->direita);
    delete raiz;
}

NoAVL* construirIndiceAVL(const vector<Produto>& produtos) {
    NoAVL* raiz = nullptr;

    for (int i = 0; i < (int)produtos.size(); i++) {
        raiz = inserirAVL(raiz, produtos[i].codigo, i);
    }

    return raiz;
}

void reconstruirIndiceAVL(NoAVL*& raiz, const vector<Produto>& produtos) {
    liberarAVL(raiz);
    raiz = construirIndiceAVL(produtos);
}

bool codigoDisponivel(NoAVL* indiceAVL, int codigo, int indiceAtual = -1) {
    int indiceEncontrado = buscarIndiceAVL(indiceAVL, codigo);
    return indiceEncontrado == -1 || indiceEncontrado == indiceAtual;
}

bool iniciaCom(const string& texto, const string& prefixo) {
    return texto.size() >= prefixo.size() && texto.compare(0, prefixo.size(), prefixo) == 0;
}

vector<IndiceNome> construirIndiceNomeOrdenado(const vector<Produto>& produtos) {
    vector<IndiceNome> indice;

    for (int i = 0; i < (int)produtos.size(); i++) {
        indice.push_back({minusculo(produtos[i].nome), produtos[i].codigo, i});
    }

    sort(indice.begin(), indice.end(), [](const IndiceNome& a, const IndiceNome& b) {
        if (a.nome != b.nome) return a.nome < b.nome;
        return a.codigo < b.codigo;
    });

    return indice;
}

int buscaBinariaPrimeiroNome(const vector<IndiceNome>& indice, const string& busca) {
    int esquerda = 0;
    int direita = (int)indice.size() - 1;
    int resultado = (int)indice.size();

    while (esquerda <= direita) {
        int meio = esquerda + (direita - esquerda) / 2;

        if (indice[meio].nome >= busca) {
            resultado = meio;
            direita = meio - 1;
        } else {
            esquerda = meio + 1;
        }
    }

    return resultado;
}

vector<Produto> carregarProdutos() {
    vector<Produto> produtos;
    ifstream arquivo(ARQUIVO, ios::binary);

    if (!arquivo) return produtos;

    Produto p;
    while (arquivo.read(reinterpret_cast<char*>(&p), sizeof(Produto))) {
        produtos.push_back(p);
    }

    return produtos;
}

void salvarProdutos(const vector<Produto>& produtos) {
    ofstream arquivo(ARQUIVO, ios::binary | ios::trunc);

    for (const Produto& p : produtos) {
        arquivo.write(reinterpret_cast<const char*>(&p), sizeof(Produto));
    }
}

void mostrarProduto(const Produto& p) {
    cout << "----------------------------------------\n";
    cout << "Codigo: " << p.codigo << "\n";
    cout << "Nome: " << p.nome << "\n";
    cout << "Categoria: " << p.categoria << "\n";
    cout << fixed << setprecision(2);
    cout << "Preco: R$ " << p.preco << "\n";
    cout << "Estoque: " << p.estoque << "\n";
    cout << "Avaliacao: " << p.avaliacao << "\n";
    cout << "Descricao: " << p.descricao << "\n";
}

void cadastrarProduto(vector<Produto>& produtos, NoAVL* indiceAVL) {
    Produto p{};
    string texto;

    cout << "Codigo do produto: ";
    cin >> p.codigo;
    limparBuffer();

    if (!codigoDisponivel(indiceAVL, p.codigo)) {
        cout << "Erro: ja existe produto com esse codigo.\n";
        return;
    }

    cout << "Nome: ";
    getline(cin, texto);
    copiarTexto(p.nome, 60, texto);

    cout << "Categoria: ";
    getline(cin, texto);
    copiarTexto(p.categoria, 40, texto);

    cout << "Preco: ";
    cin >> p.preco;

    cout << "Quantidade em estoque: ";
    cin >> p.estoque;

    cout << "Avaliacao de 0 a 5: ";
    cin >> p.avaliacao;
    limparBuffer();

    cout << "Descricao resumida: ";
    getline(cin, texto);
    copiarTexto(p.descricao, 120, texto);

    produtos.push_back(p);
    salvarProdutos(produtos);
    cout << "Produto cadastrado com sucesso!\n";
}

void listarProdutos(const vector<Produto>& produtos) {
    if (produtos.empty()) {
        cout << "Nenhum produto cadastrado.\n";
        return;
    }

    for (const Produto& p : produtos) {
        mostrarProduto(p);
    }
}

bool editarCodigo(vector<Produto>& produtos, NoAVL* indiceAVL, int pos) {
    int novoCodigo;

    cout << "Novo codigo: ";
    cin >> novoCodigo;

    if (!codigoDisponivel(indiceAVL, novoCodigo, pos)) {
        cout << "Erro: ja existe produto com esse codigo.\n";
        return false;
    }

    produtos[pos].codigo = novoCodigo;
    return true;
}

bool editarNome(Produto& produto) {
    string texto;

    limparBuffer();
    cout << "Novo nome: ";
    getline(cin, texto);
    copiarTexto(produto.nome, 60, texto);
    return true;
}

bool editarCategoria(Produto& produto) {
    string texto;

    limparBuffer();
    cout << "Nova categoria: ";
    getline(cin, texto);
    copiarTexto(produto.categoria, 40, texto);
    return true;
}

bool editarPreco(Produto& produto) {
    cout << "Novo preco: ";
    cin >> produto.preco;
    return true;
}

bool editarEstoque(Produto& produto) {
    cout << "Novo estoque: ";
    cin >> produto.estoque;
    return true;
}

bool editarAvaliacao(Produto& produto) {
    cout << "Nova avaliacao de 0 a 5: ";
    cin >> produto.avaliacao;
    return true;
}

bool editarDescricao(Produto& produto) {
    string texto;

    limparBuffer();
    cout << "Nova descricao resumida: ";
    getline(cin, texto);
    copiarTexto(produto.descricao, 120, texto);
    return true;
}

bool editarTodasInformacoes(vector<Produto>& produtos, NoAVL* indiceAVL, int pos) {
    Produto& produto = produtos[pos];
    string texto;
    int novoCodigo;

    cout << "Novo codigo: ";
    cin >> novoCodigo;

    if (!codigoDisponivel(indiceAVL, novoCodigo, pos)) {
        cout << "Erro: ja existe produto com esse codigo.\n";
        return false;
    }

    produto.codigo = novoCodigo;
    limparBuffer();

    cout << "Novo nome: ";
    getline(cin, texto);
    copiarTexto(produto.nome, 60, texto);

    cout << "Nova categoria: ";
    getline(cin, texto);
    copiarTexto(produto.categoria, 40, texto);

    cout << "Novo preco: ";
    cin >> produto.preco;

    cout << "Novo estoque: ";
    cin >> produto.estoque;

    cout << "Nova avaliacao de 0 a 5: ";
    cin >> produto.avaliacao;
    limparBuffer();

    cout << "Nova descricao resumida: ";
    getline(cin, texto);
    copiarTexto(produto.descricao, 120, texto);

    return true;
}

void editarInformacoes(vector<Produto>& produtos, NoAVL* indiceAVL) {
    int codigo;
    int opcao;
    bool alterou = false;

    if (produtos.empty()) {
        cout << "Nenhum produto cadastrado.\n";
        return;
    }

    cout << "Codigo do produto para editar: ";
    cin >> codigo;

    int pos = buscarIndiceAVL(indiceAVL, codigo);
    if (pos == -1) {
        cout << "Produto nao encontrado.\n";
        return;
    }

    do {
        cout << "\n===== EDITAR INFORMACOES =====\n";
        mostrarProduto(produtos[pos]);
        cout << "1 - Codigo\n";
        cout << "2 - Nome\n";
        cout << "3 - Categoria\n";
        cout << "4 - Preco\n";
        cout << "5 - Estoque\n";
        cout << "6 - Avaliacao\n";
        cout << "7 - Descricao\n";
        cout << "8 - Editar todas as informacoes\n";
        cout << "0 - Voltar\n";
        cout << "Escolha: ";
        cin >> opcao;

        switch (opcao) {
            case 1: alterou = editarCodigo(produtos, indiceAVL, pos) || alterou; break;
            case 2: alterou = editarNome(produtos[pos]) || alterou; break;
            case 3: alterou = editarCategoria(produtos[pos]) || alterou; break;
            case 4: alterou = editarPreco(produtos[pos]) || alterou; break;
            case 5: alterou = editarEstoque(produtos[pos]) || alterou; break;
            case 6: alterou = editarAvaliacao(produtos[pos]) || alterou; break;
            case 7: alterou = editarDescricao(produtos[pos]) || alterou; break;
            case 8: alterou = editarTodasInformacoes(produtos, indiceAVL, pos) || alterou; break;
            case 0: break;
            default: cout << "Opcao invalida.\n";
        }
    } while (opcao != 0);

    if (alterou) {
        salvarProdutos(produtos);
        cout << "Informacoes editadas com sucesso!\n";
    }
}

void limparProdutos(vector<Produto>& produtos) {
    char confirmacao;

    if (produtos.empty()) {
        cout << "Nenhum produto cadastrado para limpar.\n";
        return;
    }

    cout << "Confirma limpar todos os produtos? (s/n): ";
    cin >> confirmacao;

    if (confirmacao == 's' || confirmacao == 'S') {
        produtos.clear();
        salvarProdutos(produtos);
        cout << "Produtos limpos com sucesso!\n";
    } else {
        cout << "Operacao cancelada.\n";
    }
}

void buscarPorNome(const vector<Produto>& produtos) {
    string busca;
    bool encontrou = false;

    if (produtos.empty()) {
        cout << "Nenhum produto cadastrado.\n";
        return;
    }

    limparBuffer();
    cout << "Digite o inicio do nome: ";
    getline(cin, busca);
    busca = minusculo(busca);

    if (busca.empty()) {
        cout << "Busca vazia nao permitida.\n";
        return;
    }

    vector<IndiceNome> indiceNome = construirIndiceNomeOrdenado(produtos);

    auto inicio = chrono::high_resolution_clock::now();
    int posIndice = buscaBinariaPrimeiroNome(indiceNome, busca);

    while (posIndice < (int)indiceNome.size() && iniciaCom(indiceNome[posIndice].nome, busca)) {
        mostrarProduto(produtos[indiceNome[posIndice].indice]);
        encontrou = true;
        posIndice++;
    }

    auto fim = chrono::high_resolution_clock::now();
    auto tempo = chrono::duration_cast<chrono::microseconds>(fim - inicio).count();

    if (!encontrou) cout << "Nenhum produto encontrado.\n";
    cout << "Tempo da busca binaria: " << tempo << " microssegundos.\n";
}

void filtrarCategoria(const vector<Produto>& produtos) {
    string categoria;
    bool encontrou = false;

    limparBuffer();
    cout << "Digite a categoria: ";
    getline(cin, categoria);
    categoria = minusculo(categoria);

    for (const Produto& p : produtos) {
        if (minusculo(p.categoria) == categoria) {
            mostrarProduto(p);
            encontrou = true;
        }
    }

    if (!encontrou) cout << "Nenhum produto nessa categoria.\n";
}

void filtrarPreco(const vector<Produto>& produtos) {
    double minPreco, maxPreco;
    bool encontrou = false;

    cout << "Preco minimo: ";
    cin >> minPreco;
    cout << "Preco maximo: ";
    cin >> maxPreco;

    for (const Produto& p : produtos) {
        if (p.preco >= minPreco && p.preco <= maxPreco) {
            mostrarProduto(p);
            encontrou = true;
        }
    }

    if (!encontrou) cout << "Nenhum produto nessa faixa de preco.\n";
}

void quickSortPreco(vector<Produto>& v, int inicio, int fim) {
    if (inicio >= fim) return;

    double pivo = v[(inicio + fim) / 2].preco;
    int i = inicio, j = fim;

    while (i <= j) {
        while (v[i].preco < pivo) i++;
        while (v[j].preco > pivo) j--;

        if (i <= j) {
            swap(v[i], v[j]);
            i++;
            j--;
        }
    }

    quickSortPreco(v, inicio, j);
    quickSortPreco(v, i, fim);
}

void quickSortAvaliacao(vector<Produto>& v, int inicio, int fim) {
    if (inicio >= fim) return;

    double pivo = v[(inicio + fim) / 2].avaliacao;
    int i = inicio, j = fim;

    while (i <= j) {
        while (v[i].avaliacao > pivo) i++;
        while (v[j].avaliacao < pivo) j--;

        if (i <= j) {
            swap(v[i], v[j]);
            i++;
            j--;
        }
    }

    quickSortAvaliacao(v, inicio, j);
    quickSortAvaliacao(v, i, fim);
}

void ordenarPorPreco(const vector<Produto>& produtos) {
    vector<Produto> copia = produtos;

    auto inicio = chrono::high_resolution_clock::now();
    if (!copia.empty()) quickSortPreco(copia, 0, copia.size() - 1);
    auto fim = chrono::high_resolution_clock::now();

    listarProdutos(copia);

    auto tempo = chrono::duration_cast<chrono::microseconds>(fim - inicio).count();
    cout << "Tempo da ordenacao por preco: " << tempo << " microssegundos.\n";
}

void rankingAvaliacao(const vector<Produto>& produtos) {
    vector<Produto> copia = produtos;

    auto inicio = chrono::high_resolution_clock::now();
    if (!copia.empty()) quickSortAvaliacao(copia, 0, copia.size() - 1);
    auto fim = chrono::high_resolution_clock::now();

    cout << "\n===== RANKING POR AVALIACAO =====\n";
    int limite = min(5, (int)copia.size());

    for (int i = 0; i < limite; i++) {
        cout << i + 1 << " lugar:\n";
        mostrarProduto(copia[i]);
    }

    auto tempo = chrono::duration_cast<chrono::microseconds>(fim - inicio).count();
    cout << "Tempo da ordenacao do ranking: " << tempo << " microssegundos.\n";
}

void imprimirProdutoResumo(const Produto& p, int posicao) {
    cout << left << setw(5) << posicao
         << setw(10) << p.codigo
         << setw(28) << p.nome
         << setw(14) << p.categoria
         << right << setw(10) << fixed << setprecision(2) << p.preco
         << setw(10) << p.estoque
         << setw(12) << p.avaliacao << "\n";
}

void imprimirCabecalhoRanking() {
    cout << left << setw(5) << "#"
         << setw(10) << "Codigo"
         << setw(28) << "Nome"
         << setw(14) << "Categoria"
         << right << setw(10) << "Preco"
         << setw(10) << "Estoque"
         << setw(12) << "Avaliacao" << "\n";
    cout << string(89, '-') << "\n";
}

void gerarRankingProdutos(const vector<Produto>& produtos) {
    vector<Produto> ranking = produtos;

    sort(ranking.begin(), ranking.end(), [](const Produto& a, const Produto& b) {
        if (a.avaliacao != b.avaliacao) return a.avaliacao > b.avaliacao;
        if (a.estoque != b.estoque) return a.estoque > b.estoque;
        return a.preco < b.preco;
    });

    cout << "\n===== RANKING GERAL DE PRODUTOS =====\n";
    cout << "Criterios: maior avaliacao, maior estoque e menor preco.\n";
    imprimirCabecalhoRanking();

    int limite = min(10, (int)ranking.size());
    for (int i = 0; i < limite; i++) {
        imprimirProdutoResumo(ranking[i], i + 1);
    }
}

void adicionarResultado(vector<ResultadoDesempenho>& resultados, const string& operacao,
                        const string& algoritmo, int encontrados, int repeticoes,
                        chrono::high_resolution_clock::time_point inicio,
                        chrono::high_resolution_clock::time_point fim) {
    long long tempo = chrono::duration_cast<chrono::nanoseconds>(fim - inicio).count();
    resultados.push_back({operacao, algoritmo, encontrados, repeticoes, tempo});
}

void imprimirRelatorioDesempenho(const vector<ResultadoDesempenho>& resultados) {
    cout << "\n===== RELATORIO COMPARATIVO DE DESEMPENHO =====\n";
    cout << left << setw(28) << "Operacao"
         << setw(30) << "Algoritmo/Estrutura"
         << right << setw(12) << "Resultados"
         << setw(12) << "Repeticoes"
         << setw(16) << "Total(ns)"
         << setw(16) << "Media(ns)" << "\n";
    cout << string(114, '-') << "\n";

    for (const ResultadoDesempenho& r : resultados) {
        double media = r.repeticoes > 0 ? (double)r.tempoTotal / r.repeticoes : 0.0;

        cout << left << setw(28) << r.operacao
             << setw(30) << r.algoritmo
             << right << setw(12) << r.resultados
             << setw(12) << r.repeticoes
             << setw(16) << r.tempoTotal
             << setw(16) << fixed << setprecision(4) << media << "\n";
    }
}

void gerarRankingERelatorio(const vector<Produto>& produtos, NoAVL* indiceAVL) {
    if (produtos.empty()) {
        cout << "Nenhum produto cadastrado para gerar ranking e relatorio.\n";
        return;
    }

    const int quantidadeProdutos = (int)produtos.size();
    const int repeticoesConsulta = quantidadeProdutos <= 100 ? 100000 : 10000;
    const int repeticoesOrdenacao = quantidadeProdutos <= 100 ? 10000 : 1000;
    vector<ResultadoDesempenho> resultados;

    const Produto& produtoReferencia = produtos.back();
    int codigoReferencia = produtoReferencia.codigo;
    string nomeReferencia = minusculo(produtoReferencia.nome);
    string prefixoNome = nomeReferencia.substr(0, min(3, (int)nomeReferencia.size()));
    string categoriaReferencia = minusculo(produtoReferencia.categoria);
    double menorPreco = produtos[0].preco;
    double maiorPreco = produtos[0].preco;

    for (const Produto& p : produtos) {
        menorPreco = min(menorPreco, p.preco);
        maiorPreco = max(maiorPreco, p.preco);
    }

    vector<IndiceNome> indiceNome = construirIndiceNomeOrdenado(produtos);
    int encontrados = 0;

    volatile int acumuladorCodigoLinear = 0;
    auto inicio = chrono::high_resolution_clock::now();
    for (int i = 0; i < repeticoesConsulta; i++) {
        int atual = 0;

        for (int j = 0; j < (int)produtos.size(); j++) {
            if (produtos[j].codigo == codigoReferencia) {
                atual = 1;
                break;
            }
        }

        acumuladorCodigoLinear += atual;
    }
    auto fim = chrono::high_resolution_clock::now();
    encontrados = acumuladorCodigoLinear / repeticoesConsulta;
    adicionarResultado(resultados, "Consulta por codigo", "Busca linear", encontrados,
                       repeticoesConsulta, inicio, fim);

    volatile int acumuladorCodigoAVL = 0;
    inicio = chrono::high_resolution_clock::now();
    for (int i = 0; i < repeticoesConsulta; i++) {
        acumuladorCodigoAVL += buscarIndiceAVL(indiceAVL, codigoReferencia) != -1 ? 1 : 0;
    }
    fim = chrono::high_resolution_clock::now();
    encontrados = acumuladorCodigoAVL / repeticoesConsulta;
    adicionarResultado(resultados, "Consulta por codigo", "Arvore AVL", encontrados,
                       repeticoesConsulta, inicio, fim);

    volatile int acumuladorNomeLinear = 0;
    inicio = chrono::high_resolution_clock::now();
    for (int i = 0; i < repeticoesConsulta; i++) {
        int atual = 0;

        for (const Produto& p : produtos) {
            if (iniciaCom(minusculo(p.nome), prefixoNome)) atual++;
        }

        acumuladorNomeLinear += atual;
    }
    fim = chrono::high_resolution_clock::now();
    encontrados = acumuladorNomeLinear / repeticoesConsulta;
    adicionarResultado(resultados, "Consulta por nome", "Busca linear", encontrados,
                       repeticoesConsulta, inicio, fim);

    volatile int acumuladorNomeBinario = 0;
    inicio = chrono::high_resolution_clock::now();
    for (int i = 0; i < repeticoesConsulta; i++) {
        int atual = 0;
        int posIndice = buscaBinariaPrimeiroNome(indiceNome, prefixoNome);

        while (posIndice < (int)indiceNome.size() && iniciaCom(indiceNome[posIndice].nome, prefixoNome)) {
            atual++;
            posIndice++;
        }

        acumuladorNomeBinario += atual;
    }
    fim = chrono::high_resolution_clock::now();
    encontrados = acumuladorNomeBinario / repeticoesConsulta;
    adicionarResultado(resultados, "Consulta por nome", "Busca binaria", encontrados,
                       repeticoesConsulta, inicio, fim);

    volatile int acumuladorCategoria = 0;
    inicio = chrono::high_resolution_clock::now();
    for (int i = 0; i < repeticoesConsulta; i++) {
        int atual = 0;

        for (const Produto& p : produtos) {
            if (minusculo(p.categoria) == categoriaReferencia) atual++;
        }

        acumuladorCategoria += atual;
    }
    fim = chrono::high_resolution_clock::now();
    encontrados = acumuladorCategoria / repeticoesConsulta;
    adicionarResultado(resultados, "Filtro categoria", "Busca linear", encontrados,
                       repeticoesConsulta, inicio, fim);

    volatile int acumuladorPreco = 0;
    inicio = chrono::high_resolution_clock::now();
    for (int i = 0; i < repeticoesConsulta; i++) {
        int atual = 0;

        for (const Produto& p : produtos) {
            if (p.preco >= menorPreco && p.preco <= maiorPreco) atual++;
        }

        acumuladorPreco += atual;
    }
    fim = chrono::high_resolution_clock::now();
    encontrados = acumuladorPreco / repeticoesConsulta;
    adicionarResultado(resultados, "Filtro preco", "Busca linear", encontrados,
                       repeticoesConsulta, inicio, fim);

    volatile int controleOrdenacaoPreco = 0;
    inicio = chrono::high_resolution_clock::now();
    for (int i = 0; i < repeticoesOrdenacao; i++) {
        vector<Produto> copia = produtos;
        if (!copia.empty()) {
            quickSortPreco(copia, 0, copia.size() - 1);
            controleOrdenacaoPreco += copia[0].codigo;
        }
    }
    fim = chrono::high_resolution_clock::now();
    adicionarResultado(resultados, "Ordenacao por preco", "QuickSort", (int)produtos.size(),
                       repeticoesOrdenacao, inicio, fim);

    volatile int controleOrdenacaoAvaliacao = 0;
    inicio = chrono::high_resolution_clock::now();
    for (int i = 0; i < repeticoesOrdenacao; i++) {
        vector<Produto> copia = produtos;
        if (!copia.empty()) {
            quickSortAvaliacao(copia, 0, copia.size() - 1);
            controleOrdenacaoAvaliacao += copia[0].codigo;
        }
    }
    fim = chrono::high_resolution_clock::now();
    adicionarResultado(resultados, "Ordenacao avaliacao", "QuickSort", (int)produtos.size(),
                       repeticoesOrdenacao, inicio, fim);

    volatile int controleOrdenacaoNome = 0;
    inicio = chrono::high_resolution_clock::now();
    for (int i = 0; i < repeticoesOrdenacao; i++) {
        vector<IndiceNome> copia = construirIndiceNomeOrdenado(produtos);
        if (!copia.empty()) controleOrdenacaoNome += copia[0].codigo;
        encontrados = (int)copia.size();
    }
    fim = chrono::high_resolution_clock::now();
    adicionarResultado(resultados, "Ordenacao por nome", "std::sort", encontrados,
                       repeticoesOrdenacao, inicio, fim);

    gerarRankingProdutos(produtos);
    imprimirRelatorioDesempenho(resultados);
    cout << "\nBase analisada: " << produtos.size() << " produto(s).\n";
    cout << "Consultas usam o ultimo produto cadastrado como referencia.\n";
}

void menu() {
    vector<Produto> produtos = carregarProdutos();
    NoAVL* indiceAVL = construirIndiceAVL(produtos);
    int opcao;

    do {
        cout << "\n========================================\n";
        cout << " GERENCIADOR DE PRODUTOS E-COMMERCE\n";
        cout << "========================================\n";
        cout << "1 - Cadastrar produto\n";
        cout << "2 - Listar produtos\n";
        cout << "3 - Buscar por nome\n";
        cout << "4 - Filtrar por categoria\n";
        cout << "5 - Filtrar por faixa de preco\n";
        cout << "6 - Ordenar por preco\n";
        cout << "7 - Editar informacoes\n";
        cout << "8 - Ranking por avaliacao\n";
        cout << "9 - Ranking e relatorio de desempenho\n";
        cout << "10 - Limpar produtos\n";
        cout << "0 - Sair\n";
        cout << "Escolha: ";
        cin >> opcao;

        switch (opcao) {
            case 1:
                cadastrarProduto(produtos, indiceAVL);
                reconstruirIndiceAVL(indiceAVL, produtos);
                break;
            case 2: listarProdutos(produtos); break;
            case 3: buscarPorNome(produtos); break;
            case 4: filtrarCategoria(produtos); break;
            case 5: filtrarPreco(produtos); break;
            case 6: ordenarPorPreco(produtos); break;
            case 7:
                editarInformacoes(produtos, indiceAVL);
                reconstruirIndiceAVL(indiceAVL, produtos);
                break;
            case 8: rankingAvaliacao(produtos); break;
            case 9: gerarRankingERelatorio(produtos, indiceAVL); break;
            case 10:
                limparProdutos(produtos);
                reconstruirIndiceAVL(indiceAVL, produtos);
                break;
            case 0:
                cout << "Saindo...\n";
                break;
            default: cout << "Opcao invalida.\n";
        }
    } while (opcao != 0);

    liberarAVL(indiceAVL);
}

int main() {
    menu();
    return 0;
}
