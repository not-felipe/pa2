#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

using namespace std;

struct Registro {
  string data;
  map<string, int> ocorrencias;
};

void lerArquivoCSV(string nomeArquivo, map<string, vector<Registro>>& registros) {
    if (nomeArquivo.find(".csv") == string::npos) {
        nomeArquivo += ".csv";
    }

    ifstream arquivo(nomeArquivo);

    if (!arquivo.is_open()) {
        cerr << "Erro ao abrir o arquivo " << nomeArquivo << endl;
        return;
    }

    // Extrair a data do nome do arquivo
    size_t pos = nomeArquivo.find_last_of("/");
    string dataArquivo = nomeArquivo.substr(pos + 1, 8); // Assume que o formato do nome do arquivo é DDMMYYYY

    string linha;
    while (getline(arquivo, linha)) {
        cout << "Linha lida: " << linha << endl;
        stringstream ss(linha);
        string nome;
        getline(ss, nome, ',');
        Registro registro;
        registro.data = dataArquivo;
        string chave;
        string valor;
        while (getline(ss, chave, ',')) {
            if (!getline(ss, valor, ',')) {
                break;
            }
            registro.ocorrencias[chave] = stoi(valor);
        }
        registros[nome].push_back(registro);
    }

    cout << "Arquivo " << nomeArquivo << " lido com sucesso." << endl;
}

void gerarRelatorioPorCrianca(const map<string, vector<Registro>>& registros) {
    string nome;
    cout << "Digite o nome da criança: ";
    cin.ignore();
    getline(cin, nome);

    auto it = registros.find(nome);
    if (it == registros.end()) {
        cout << "Criança não encontrada." << endl;
        return;
    }

    const vector<Registro>& registrosCrianca = it->second;

    cout << "Relatório para a criança: " << nome << endl;

    for (const Registro& registro : registrosCrianca) {
        cout << "Data: " << registro.data << endl;

        for (const auto& ocorrencia : registro.ocorrencias) {
            cout << setw(15) << ocorrencia.first << ": " << ocorrencia.second << endl;
        }
        cout << endl;
    }
}

void gerarRelatorioGeral(const map<string, vector<Registro>>& registros, int op) {

    string quesito;

    switch(op){
        case 1:
          quesito = "penalti";
          break;
        case 2:
          quesito = "falta";
          break;
        case 3:
          quesito = "escanteio";
          break;
        case 4:
          quesito = "gol";
          break;
        case 5:
          quesito = "cruzamento";
          break;
        case 6:
          quesito = "cartao amarelo";
          break;
        case 7:
          quesito = "cartao vermelho";
          break;
        case 8:
          quesito = "drible";
          break;
        case 9:
          quesito = "desarme";
          break;
        case 10:
          quesito = "cabeceio";
          break;
        case 11: 
          quesito = "assistencia";
          break;
    }
    cout << "Relatório de " + quesito + " por criança:" << endl;

    bool encontrado = false; // Verificar se há registros do quesito escolhido para alguma criança

    for (const auto& par : registros) {
        const string& nome = par.first;
        const vector<Registro>& registrosCrianca = par.second;

        for (const Registro& registro : registrosCrianca) {
            auto itQuesito = registro.ocorrencias.find(quesito);
            if (itQuesito != registro.ocorrencias.end()) {
                if (!encontrado) {
                    encontrado = true;
                }

                // Formatar a data como DD/MM/YYYY
                string dataFormatada = registro.data.substr(0, 2) + "/" + registro.data.substr(2, 2) + "/" + registro.data.substr(4, 4);
                cout << "Criança: " << nome << endl;
                cout << "Data: " << dataFormatada << endl;
                cout << "  - " + quesito + ": " << itQuesito->second << endl;
                cout << endl;
            }
        }
    }

    if (!encontrado) {
        cout << "Não foram encontrados registros de " + quesito + " para nenhuma criança." << endl;
    }
}

void salvarRegistrosEmArquivoBinario(
    const map<string, vector<Registro>> &registros) {
  ofstream arquivoBinario("indice.dat", ios::binary);
  if (!arquivoBinario.is_open()) {
    cerr << "Erro ao abrir o arquivo binário." << endl;
    return;
  }

  for (const auto &par : registros) {
    const string &nome = par.first;
    const vector<Registro> &registrosCrianca = par.second;

    for (const Registro &registro : registrosCrianca) {
      // Escrever nome
      int tamanhoNome = nome.size();
      arquivoBinario.write(reinterpret_cast<const char *>(&tamanhoNome),
                           sizeof(int));
      arquivoBinario.write(nome.c_str(), tamanhoNome);

      // Escrever data
      int tamanhoData = registro.data.size();
      arquivoBinario.write(reinterpret_cast<const char *>(&tamanhoData),
                           sizeof(int));
      arquivoBinario.write(registro.data.c_str(), tamanhoData);

      // Escrever número de ocorrências
      int numOcorrencias = registro.ocorrencias.size();
      arquivoBinario.write(reinterpret_cast<const char *>(&numOcorrencias),
                           sizeof(int));

      // Escrever ocorrências
      for (const auto &ocorrencia : registro.ocorrencias) {
        const string &chave = ocorrencia.first;
        int tamanhoChave = chave.size();
        arquivoBinario.write(reinterpret_cast<const char *>(&tamanhoChave),
                             sizeof(int));
        arquivoBinario.write(chave.c_str(), tamanhoChave);
        arquivoBinario.write(reinterpret_cast<const char *>(&ocorrencia.second),
                             sizeof(int));
      }
    }
  }

  cout << "Registros salvos em arquivo binário com sucesso." << endl;
}

void carregarRegistrosDeArquivoBinario(
    map<string, vector<Registro>> &registros) {
  ifstream arquivoBinario("indice.dat", ios::binary);
  if (!arquivoBinario.is_open()) {
    cout << "Arquivo binário não encontrado. Será criado um novo arquivo."
         << endl;
    return;
  }

  registros.clear();

  while (true) {
    int tamanhoNome;
    arquivoBinario.read(reinterpret_cast<char *>(&tamanhoNome), sizeof(int));
    if (arquivoBinario.eof()) {
      break;
    }

    string nome;
    nome.resize(tamanhoNome);
    arquivoBinario.read(&nome[0], tamanhoNome);

    int tamanhoData;
    arquivoBinario.read(reinterpret_cast<char *>(&tamanhoData), sizeof(int));

    string data;
    data.resize(tamanhoData);
    arquivoBinario.read(&data[0], tamanhoData);

    int numOcorrencias;
    arquivoBinario.read(reinterpret_cast<char *>(&numOcorrencias), sizeof(int));

    Registro registro;
    registro.data = data;

    for (int i = 0; i < numOcorrencias; i++) {
      int tamanhoChave;
      arquivoBinario.read(reinterpret_cast<char *>(&tamanhoChave), sizeof(int));

      string chave;
      chave.resize(tamanhoChave);
      arquivoBinario.read(&chave[0], tamanhoChave);

      int valor;
      arquivoBinario.read(reinterpret_cast<char *>(&valor), sizeof(int));

      registro.ocorrencias[chave] = valor;
    }

    registros[nome].push_back(registro);
  }

  cout << "Registros carregados do arquivo binário com sucesso." << endl;
}

int main() {
  map<string, vector<Registro>> registros;

  int opcao;
  while (true) {
    cout << "Escolha uma opção:" << endl;
    cout << "1 - Ler arquivo CSV" << endl;
    cout << "2 - Gerar relatório por criança" << endl;
    cout << "3 - Gerar relatório geral por quesito" << endl;
    cout << "4 - Salvar registros em arquivo binário" << endl;
    cout << "5 - Carregar registros de um arquivo binario" << endl;
    cout << "0 - Sair" << endl;
    cout << "Opção: ";
    cin >> opcao;

    switch (opcao) {
    case 1: {
      string nomeArquivo;
      cout << "Digite o nome do arquivo CSV: ";
      cin.ignore();
      getline(cin, nomeArquivo);
      lerArquivoCSV(nomeArquivo, registros);
      break;
    }
    case 2:
      gerarRelatorioPorCrianca(registros);
      break;
    case 3:
      {
        int op = 8;
        cout << "ESCOLHA A OPÇÃO PARA GERAR O RELATÓRIO:\n[1] Penalti\n[2] Falta\n[3] Escanteio\n[4] Gol\n[5] Cruzamento\n[6] Cartão Amarelo\n[7] Cartão Vermelho\n[8] Drible\n[9] Desarme\n[10] Cabeceio\n[11] Assistência\n[->] ";
        cin >> op;
        gerarRelatorioGeral(registros, op);
      }
    case 4:
      salvarRegistrosEmArquivoBinario(registros);
      break;
    case 5:
      // Carregar registros do arquivo binário, se existir
      carregarRegistrosDeArquivoBinario(registros);
      break;
    case 0:
      cout << "Encerrando o programa." << endl;
      return 0;
    default:
      cout << "Opção inválida. Tente novamente." << endl;
      break;
    }

    cout << endl;
  }

  return 0;
}
