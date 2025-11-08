/*
 * Simulação de Coleta de Dados do Solo para Arduino
 * * Este código simula a coleta de dados de sensores (temperatura, umidade, salinidade)
 * a cada 15 minutos. Ele armazena os dados em uma lista encadeada.
 * A cada 4 horas (16 amostras), ele processa os dados:
 * 1. Remove a amostra com a MAIOR e a MENOR temperatura.
 * 2. Calcula a média dos 14 valores restantes.
 * 3. Toma uma decisão de irrigação.
 * 4. Imprime um relatório na porta Serial.
 */

// --- Estruturas de Dados ---

// Estrutura para armazenar cada amostra de dados
struct SampleData {
  float temperature;
  float humidity;
  float salinity;
};

// Estrutura para o nó da lista encadeada
struct Node {
  SampleData data;
  Node* next;
};

// --- Variáveis Globais ---

Node* dataListHead = nullptr; // Ponteiro para o início da lista
int sampleCount = 0;          // Contador de amostras
unsigned long lastSampleTime = 0; // Tempo da última coleta

// Intervalo de 15 minutos (em milissegundos)
// const unsigned long SAMPLE_INTERVAL = 15 * 60 * 1000; 
// Para fins de teste rápido, usaremos 10 segundos:
const unsigned long SAMPLE_INTERVAL = 10000; // 10 segundos

const int SAMPLES_PER_CYCLE = 16; // 4 horas / 15 minutos = 16 amostras

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // Aguarda a porta serial conectar
  }
  Serial.println("--- Sistema de Monitoramento de Solo Iniciado ---");
  Serial.print("Coletando 1 amostra a cada ");
  Serial.print(SAMPLE_INTERVAL / 1000);
  Serial.println(" segundos (simulação).");
}

void loop() {
  // Verifica se é hora de coletar uma nova amostra
  if (millis() - lastSampleTime >= SAMPLE_INTERVAL) {
    lastSampleTime = millis(); // Reseta o temporizador
    
    collectSample();
    
    Serial.print("Amostra ");
    Serial.print(sampleCount);
    Serial.print(" coletada. (Temp: ");
    Serial.print(dataListHead->data.temperature);
    Serial.println(" C)");

    // Se completamos o ciclo de 4 horas (16 amostras)
    if (sampleCount == SAMPLES_PER_CYCLE) {
      Serial.println("\n--- Ciclo de 4 horas concluído. Processando dados... ---");
      processAndReportData();
      
      // Limpa a lista para o próximo ciclo
      clearList(); 
      Serial.println("--- Lista de dados limpa. Iniciando novo ciclo. ---\n");
    }
  }
}

// --- Funções de Simulação de Sensor ---

float readSimulatedTemperature() {
  // Simula temperatura entre 15.0 e 35.0 C
  return 15.0 + (rand() % 200) / 10.0;
}

float readSimulatedHumidity() {
  // Simula umidade entre 30.0% e 90.0%
  return 30.0 + (rand() % 600) / 10.0;
}

float readSimulatedSalinity() {
  // Simula salinidade (ex: dS/m) entre 0.5 e 4.0
  return 0.5 + (rand() % 35) / 10.0;
}

// --- Funções da Lógica Principal ---

void collectSample() {
  // 1. Coleta dados (simulados)
  SampleData newSample;
  newSample.temperature = readSimulatedTemperature();
  newSample.humidity = readSimulatedHumidity();
  newSample.salinity = readSimulatedSalinity();

  // 2. Cria um novo nó para a lista
  // NOTA: Em sistemas embarcados, 'new' (alocação dinâmica) pode
  // causar fragmentação de memória. Para um produto real, seria 
  // mais seguro usar um array estático de 16 posições.
  // Mas, seguindo o requisito de "lista encadeada", usamos 'new'.
  Node* newNode = new Node;
  newNode->data = newSample;
  
  // 3. Adiciona o nó no início da lista
  newNode->next = dataListHead;
  dataListHead = newNode;
  
  sampleCount++;
}

void processAndReportData() {
  if (sampleCount != SAMPLES_PER_CYCLE) {
    return; // Não faz nada se não tivermos 16 amostras
  }

  // 1. Encontrar os nós com min/max (baseado na temperatura)
  Node* minNodePrev = nullptr;
  Node* maxNodePrev = nullptr;
  Node* minNode = dataListHead;
  Node* maxNode = dataListHead;
  
  Node* current = dataListHead;
  Node* prev = nullptr;

  while (current != nullptr) {
    if (current->data.temperature < minNode->data.temperature) {
      minNode = current;
      minNodePrev = prev;
    }
    if (current->data.temperature > maxNode->data.temperature) {
      maxNode = current;
      maxNodePrev = prev;
    }
    prev = current;
    current = current->next;
  }

  Serial.print("Removendo valor MÍNIMO (Temp: ");
  Serial.print(minNode->data.temperature);
  Serial.println(")");
  Serial.print("Removendo valor MÁXIMO (Temp: ");
  Serial.print(maxNode->data.temperature);
  Serial.println(")");

  // 2. Remover os nós da lista
  // Importante: Deve-se tratar o caso onde min/max são o mesmo nó
  // ou onde um deles é removido primeiro, afetando o 'prev' do outro.
  // Para simplificar, vamos apenas "pular" eles na soma
  // e deletá-los depois. (Uma remoção real é mais complexa)
  // Por simplicidade aqui, vamos recalcular a soma sem eles.

  float sumTemp = 0;
  float sumHum = 0;
  float sumSal = 0;
  int remainingCount = 0;
  
  current = dataListHead;
  while (current != nullptr) {
    // Pula o nó min e o nó max
    if (current != minNode && current != maxNode) {
      sumTemp += current->data.temperature;
      sumHum += current->data.humidity;
      sumSal += current->data.salinity;
      remainingCount++; // Deve ser 14
    }
    current = current->next;
  }

  // 3. Calcular a média
  float avgTemp = sumTemp / remainingCount;
  float avgHum = sumHum / remainingCount;
  float avgSal = sumSal / remainingCount;

  // 4. Tomar decisão de irrigação (LÓGICA DO SENSOR)
  String irrigationDecision = decideIrrigation(avgTemp, avgHum, avgSal);

  // 5. Apresentar relatório na Serial (simulando o "arquivo de texto")
  printReport(avgTemp, avgHum, avgSal, irrigationDecision, remainingCount);
}

String decideIrrigation(float temp, float hum, float sal) {
  // LÓGICA DE DECISÃO DE EXEMPLO:
  // Esta é a lógica que deve ser definida.
  // Ex: Irrigar se a umidade for baixa, mas não se a salinidade for alta.
  if (hum < 45.0) {
    if (sal > 3.0) {
      return "NAO IRRIGAR (Risco de Salinidade Alta)";
    } else {
      return "IRRIGAR (Umidade Baixa)";
    }
  } else if (hum > 80.0) {
    return "NAO IRRIGAR (Umidade Alta)";
  } else {
    return "NAO IRRIGAR (Nivel Ideal)";
  }
}

void printReport(float temp, float hum, float sal, String decision, int count) {
  Serial.println("-------------------------------------------------");
  Serial.println("RELATORIO DE PROCESSAMENTO (Ciclo de 4 Horas)");
  Serial.println("-------------------------------------------------");
  Serial.print("Amostras processadas (apos remocao min/max): ");
  Serial.println(count);
  Serial.println("\nValores Medios:");
  Serial.print("  Temperatura Media: ");
  Serial.print(temp, 2);
  Serial.println(" C");
  Serial.print("  Umidade Media:     ");
  Serial.print(hum, 2);
  Serial.println(" %");
  Serial.print("  Salinidade Media:  ");
  Serial.print(sal, 2);
  Serial.println(" dS/m (simulado)");
  Serial.println("\nDecisao de Irrigacao:");
  Serial.print("  ");
  Serial.println(decision);
  Serial.println("-------------------------------------------------");
}

void clearList() {
  Node* current = dataListHead;
  Node* nextNode = nullptr;

  while (current != nullptr) {
    nextNode = current->next; // Guarda o próximo
    delete current;           // Deleta o atual
    current = nextNode;       // Avança para o próximo
  }
  
  dataListHead = nullptr; // Reseta o ponteiro da cabeça
  sampleCount = 0;      // Reseta o contador
}