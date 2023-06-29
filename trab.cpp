#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <exception>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

using namespace std;

struct Registro {
  string data;
  map<string, int> ocorrencias;
};

void lerArquivoCSV(string nomeArquivo, map<string, vector<Registro>>& registros) {
    if (nomeArquivo.find(".csv") == string::npos)  nomeArquivo += ".csv";

    ifstream arquivo(nomeArquivo);

    if (!arquivo.is_open()) {
        cerr << "Erro ao abrir o arquivo " << nomeArquivo << endl;
        return;
    }

    size_t pos = nomeArquivo.find_last_of("/");
    string dataArquivo = nomeArquivo.substr(pos + 1, 8); 

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

string trim(const string& str) {
    size_t first = str.find_first_not_of(' ');

    if (string::npos == first) return str;
    
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

void lerArquivosCSVemLote(const string& nomeArquivoLista, map<string, vector<Registro>>& registros) {
    ifstream arquivoLista(nomeArquivoLista);

    if (!arquivoLista.is_open()) {
        cerr << "Erro ao abrir o arquivo de lista " << nomeArquivoLista << endl;
        return;
    }

    string nomeArquivo;
    int linha = 1;

    ofstream arquivoLog("log.txt", ios::app);

    while (getline(arquivoLista, nomeArquivo)) {
        nomeArquivo = trim(nomeArquivo);
        fs::path caminhoArquivo(nomeArquivo); 

        if (caminhoArquivo.extension().empty() || caminhoArquivo.extension() != ".csv") {
            caminhoArquivo += ".csv"; 
        }

        ifstream arquivoCSV(caminhoArquivo);
        if (!arquivoCSV.is_open()) {
            arquivoLog << "Erro ao abrir o arquivo " << caminhoArquivo << " na linha " << linha << endl;
            linha++;
            continue;
        }

        string linhaCSV;
        int linhaArquivoCSV = 1;

        while (getline(arquivoCSV, linhaCSV)) {
            try {
                stringstream ss(linhaCSV);
                string nome;
                getline(ss, nome, ',');
                Registro registro;
                registro.data = caminhoArquivo.filename().stem(); 

                string chave;
                string valor;
                while (getline(ss, chave, ',')) {
                    if (!getline(ss, valor, ',')) {
                        break;
                    }

                    try {
                        int ocorrencia = stoi(valor);
                        if (ocorrencia < 0) {
                            throw invalid_argument("O valor de ocorrência deve ser um número inteiro positivo.");
                        }
                        registro.ocorrencias[chave] = ocorrencia;
                    } catch (const exception& e) {
                        arquivoLog << "Problema encontrado no arquivo " << caminhoArquivo << " na linha " << linhaArquivoCSV << ": " << e.what() << endl;
                        continue;
                    }
                }

                if (registros.count(nome) > 0 && !registros[nome].empty()) {
                    arquivoLog << "A criança " << nome << " aparece mais de uma vez no arquivo " << caminhoArquivo << " na linha " << linhaArquivoCSV << " do arquivo lote.txt"<<endl;
                }
                registros[nome].push_back(registro);
            } catch (const exception& e) {
                arquivoLog << "Problema encontrado no arquivo " << caminhoArquivo << " na linha " << linhaArquivoCSV << ": " << e.what() << endl;
            }

            linhaArquivoCSV++;
        }

        arquivoCSV.close();
        linha++;
    }

    arquivoLista.close();
    arquivoLog.close();

    cout << "Processamento dos arquivos em lote concluído." << endl;
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

void gerarRelatorioGeral(const map<string, vector<Registro>>& registros) {
    vector<string> tipos; 

    for (const auto& par : registros) {
        const vector<Registro>& registrosCrianca = par.second;

        for (const Registro& registro : registrosCrianca) {
            for (const auto& ocorrencia : registro.ocorrencias) {
                const string& tipo = ocorrencia.first;

                auto it = find(tipos.begin(), tipos.end(), tipo);
                if (it == tipos.end()) {
                    tipos.push_back(tipo);
                }
            }
        }
    }

    cout << "Tipos de relatórios disponíveis" << endl;
    for (size_t i = 0; i < tipos.size(); i++) {
        cout << "[" << (i + 1) << "] " << tipos[i] << endl;
    }

    int opcao;
    cout << "Selecione 0 para sair: ";
    cin >> opcao;

    if (opcao == 0) {
        cout << "Operação cancelada." << endl;
        return;
    }

    if (opcao < 1 || opcao > static_cast<int>(tipos.size())) {
        cout << "Opção inválida." << endl;
        return;
    }

    string tipoSelecionado = tipos[opcao - 1];

    cout << "Relatório de " << tipoSelecionado << " por criança:" << endl;

    bool encontrado = false;

    for (const auto& par : registros) {
        const string& nome = par.first;
        const vector<Registro>& registrosCrianca = par.second;

        for (const Registro& registro : registrosCrianca) {
            auto itQuesito = registro.ocorrencias.find(tipoSelecionado);
            if (itQuesito != registro.ocorrencias.end()) {
                if (!encontrado) {
                    encontrado = true;
                }

                // Formatar a data como DD/MM/YYYY
                string dataFormatada = registro.data.substr(0, 2) + "/" + registro.data.substr(2, 2) + "/" + registro.data.substr(4, 4);
                cout << "Criança: " << nome << endl;
                cout << "Data: " << dataFormatada << endl;
                cout << "  - " + tipoSelecionado + ": " << itQuesito->second << endl;
                cout << endl;
            }
        }
    }

    if (!encontrado) {
        cout << "Não foram encontrados registros de " + tipoSelecionado + " para nenhuma criança." << endl;
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
      cout << "5 - Carregar registros de um arquivo binário" << endl;
      cout << "6 - Ler arquivos CSV em lote" << endl;
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
    case 3:{
        gerarRelatorioGeral(registros);
        break;
      }
    case 4:
      salvarRegistrosEmArquivoBinario(registros);
      break;
    case 5:
      carregarRegistrosDeArquivoBinario(registros);
      break;
    case 6: {
            string nomeArquivoLista;
            cout << "Digite o nome do arquivo de lista: ";
            cin.ignore();
            getline(cin, nomeArquivoLista);
            lerArquivosCSVemLote(nomeArquivoLista, registros);
            break;
        }
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
