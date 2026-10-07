#include <iostream> // Para entrada/salida estándar (cout, cerr)
#include <fstream>  // Para manejo de ficheros (ifstream, ofstream)
#include <string>   // Para manejo de cadenas (string)
#include <vector>   // Para uso de vectores (vector)
#include <unordered_map> // Tabla hash para la BH 
#include <algorithm> // Para funciones como min, max
#include <cctype>   // Para funciones de utilidad de caracteres (isspace)
#include <cmath>    // Para funciones matemáticas (fabs)
#include <iomanip>  // Para formateo de salida

using namespace std;

const double NO_DEMOSTRADO = 2.0; // Valor fuera de [-1,1] para indicar "no demostrado"

// Representa una regla de la base de conocimientos
struct Regla 
{
    string numero; // Identificador de la regla: "R1", "R2", ...
    vector<string> antecedentes; // Lista de antecedentes
    string operador; // Operador lógico entre antecedentes: "AND", "OR" o "" si solo hay uno
    string consecuente; // Consecuente de la regla
    double fc; // Factor de certeza de la regla
};

vector<Regla> BC; // Base de conocimientos
unordered_map<string,double> BH; // Base de hechos
ofstream ficheroLog; // Fichero log de la ejecución

// Función para quitar espacios en blanco al inicio y al final de la cadena
string recortar(const string& s) 
{
    size_t inicio = 0;
    while (inicio < s.size() && isspace((unsigned char)s[inicio])) 
    {
        inicio++; // Avanzar mientras haya espacios al principio
    }
    if (inicio == s.size()) 
    {
        return ""; // Si la cadena es solo espacios, devolver cadena vacía
    }
    size_t fin = s.size() - 1; // Índice final
    while (fin > inicio && isspace((unsigned char)s[fin])) 
    {
        fin--; // Retroceder mientras haya espacios al final
    }
    return s.substr(inicio, fin - inicio + 1); // Devolver la subcadena recortada
}

// Función para comprobar si subc está contenida en cadena (para ver si en los antecedentes hay " y " o " o ")
bool contieneCadena(const string& s, const string& subc) 
{
    return s.find(subc) != string::npos;
}

// Divide una cadena usando un separador tipo " y " o " o " (para separar los antecedentes)
vector<string> dividirPorOp(const string& s, const string& op) 
{
    vector<string> resultado;
    size_t inicio = 0;
    size_t pos = 0;
    while ((pos = s.find(op, inicio)) != string::npos) 
    {
        string parte = s.substr(inicio, pos - inicio); // Extraer la parte antes del separador
        resultado.push_back(recortar(parte)); // Recortar y añadir al resultado
        inicio = pos + op.size(); // Avanzar "inicio" hasta después del operador
    }
    string ultima = s.substr(inicio); // Última parte después del operador
    ultima = recortar(ultima); // Recortar espacios
    if (!ultima.empty())
    {
        resultado.push_back(ultima); // Añadir la última parte si no está vacía
    }
    return resultado; // Devolver el vector de partes separadas
}

// Leer el fichero de la BC y almacenarlo en el vector BC
void procesarBC(const string& nombreFichero) 
{
    BC.clear(); // Vaciar la BC antes de cargarla por si acaso
    ifstream fichero(nombreFichero); // Abrir el fichero de la BC
    if (!fichero.is_open())
    {
        cerr << "Error abriendo BC: " << nombreFichero << endl;
        exit(1); // Terminar si no se puede abrir el fichero
    }

    string linea;
    // Primera línea: número de reglas
    if (!getline(fichero, linea))
    {
        cerr << "BC vacía o formato incorrecto\n";
        exit(1); // Error si no se puede leer la primera línea
    }
    int numeroReglas = stoi(recortar(linea)); // Leer la primera linea para saber el número de reglas

    for (int i = 0; i < numeroReglas; ++i) 
    {
        if (!getline(fichero, linea))
        {
            cerr << "Faltan reglas en " << nombreFichero << endl;
            exit(1); // Error si faltan reglas
        }

        linea = recortar(linea); // Recortar la línea leída
        if (linea.empty()) 
        { 
            --i; // Si la línea está vacía, ingorarla y no contarla
            continue; 
        }

        size_t posDosPuntos = linea.find(':'); // Buscar posición de ':', que separa número de regla del resto
        string numeroRegla = recortar(linea.substr(0, posDosPuntos)); // Extraer número de regla

        size_t posSi = linea.find("Si"); // Buscar posición de "Si"
        size_t posEntonces = linea.find("Entonces"); // Buscar posición de "Entonces"
        size_t posFC = linea.find("FC="); // Buscar posición de "FC="

        if (posSi == string::npos || posEntonces == string::npos || posFC == string::npos)
        {
            cerr << "Formato de regla incorrecto: " << linea << endl;
            exit(1); // Error si falta alguna parte de las anteriores
        }

        // Parte "Si ... Entonces ...", hasta antes de "FC"
        string condicionYConsecuente = linea.substr(posSi + 2, posFC - (posSi + 2));
        condicionYConsecuente = recortar(condicionYConsecuente); // Recortar la parte obtenida

        // Separar condición antecedentes de consecuente
        size_t posEntoncesLocal = condicionYConsecuente.find("Entonces"); // Buscar "Entonces" en la parte obtenida
        if (posEntoncesLocal == string::npos)
        {
            cerr << "Falta 'Entonces' en regla: " << linea << endl;
            exit(1); // Error si no se encuentra "Entonces"
        }

        string parteCondicion   = condicionYConsecuente.substr(0, posEntoncesLocal); // Extraer antecedentes
        string parteConsecuente = condicionYConsecuente.substr(posEntoncesLocal + string("Entonces").size()); // Extraer consecuentes después de "Entonces"

        parteCondicion   = recortar(parteCondicion); // Recortar antecedentes
        parteConsecuente = recortar(parteConsecuente); // Recortar consecuentes

        // Quitar coma al final del consecuente
        size_t coma = parteConsecuente.find(','); // Buscar coma en el consecuente 
        if (coma != string::npos) 
        {
            parteConsecuente = recortar(parteConsecuente.substr(0, coma)); // Quitar texto después de la coma
        }

        // Obtener FC
        string fcCadena = linea.substr(posFC + 3); // Extraer texto después de "FC="
        fcCadena = recortar(fcCadena); // Recortar la cadena del FC
        double fc = stod(fcCadena); // Convertir a double

        // Analizar operador AND/OR y antecedentes
        vector<string> antecedentes;
        string operador;

        if (contieneCadena(parteCondicion, " y "))
        {
            operador = "AND"; // Indicar operador AND
            antecedentes = dividirPorOp(parteCondicion, " y "); // Dividir antecedentes por " y "
        } 
        else if (contieneCadena(parteCondicion, " o ")) 
        {
            operador = "OR"; // Indicar operador OR
            antecedentes = dividirPorOp(parteCondicion, " o "); // Dividir antecedentes por " o "
        } 
        else 
        {
            operador = ""; // Sin operador (un solo antecedente)
            if (!parteCondicion.empty())
            {
                antecedentes.push_back(parteCondicion); // Añadir el único antecedente
            }
        }

        Regla regla; // Crear la regla
        regla.numero = numeroRegla; // Asignar número de regla
        regla.antecedentes = antecedentes; // Asignar antecedentes
        regla.operador = operador; // Asignar operador
        regla.consecuente = parteConsecuente; // Asignar consecuente
        regla.fc = fc; // Asignar FC

        BC.push_back(regla); // Añadir la regla a la BC
    }

    fichero.close(); // Cerrar el fichero de la BC
}

// Leer el fichero de la BH y almacenarlo en el mapa BH
void procesarBH(const string& nombreFichero, string& objetivo) 
{
    BH.clear(); // Vaciar la BH antes de cargarla por si acaso
    ifstream fichero(nombreFichero); // Abrir el fichero de la BH
    if (!fichero.is_open())
    {
        cerr << "Error abriendo BH: " << nombreFichero << endl;
        exit(1); // Terminar si no se puede abrir el fichero
    }

    string linea;
    // Primera línea: número de hechos
    if (!getline(fichero, linea))
    {
        cerr << "BH vacía o formato incorrecto\n";
        exit(1); // Error si no se puede leer la primera línea
    }
    int numeroHechos = stoi(recortar(linea)); // Leer la primera linea para saber el número de hechos

    for (int i = 0; i < numeroHechos; ++i) 
    {
        if (!getline(fichero, linea))
        {
            cerr << "Faltan hechos en " << nombreFichero << endl;
            exit(1); // Error si faltan hechos
        }

        linea = recortar(linea); // Recortar la línea leída
        if (linea.empty()) 
        { 
            --i; // Si la línea está vacía, ingorarla y no contarla
            continue; 
        }

        size_t coma = linea.find(','); // Buscar la coma que separa hecho de FC
        if (coma == string::npos)
        {
            cerr << "Formato de hecho incorrecto: " << linea << endl;
            exit(1); // Error si no se encuentra la coma
        }

        string hecho = recortar(linea.substr(0, coma)); // Extraer el nombre del hecho antes de la coma
        string resto = recortar(linea.substr(coma + 1)); // Extraer el resto después de la coma
        size_t posFC = resto.find("FC="); // Buscar "FC=" en el resto
        if (posFC == string::npos)
        {
            cerr << "Falta 'FC=' en hecho: " << linea << endl;
            exit(1); // Error si no se encuentra "FC="
        }
        string fcCadena = recortar(resto.substr(posFC + 3)); // Extraer la cadena del FC después de "FC="
        double fc = stod(fcCadena); // Convertir a double

        BH[hecho] = fc; // Añadir el hecho y su FC a la BH
    }

    // Buscar línea "Objetivo"
    while (getline(fichero, linea)) 
    {
        linea = recortar(linea); // Recortar la línea leída
        if (linea == "Objetivo") 
        {
            break; // Parar al encontrar "Objetivo"
        }
    }

    // La siguiente línea debe ser la meta objetivo
    if (!getline(fichero, linea))
    {
        cerr << "Falta hecho objetivo en " << nombreFichero << endl;
        exit(1); // Error si no se puede leer la línea del objetivo
    }
    objetivo = recortar(linea); // Asignar el objetivo

    fichero.close(); // Cerrar el fichero de la BH
}

// Motor SBR-FC
// Comprobar si la meta ya está en la BH
bool contenidaEnBH(const string& meta) 
{
    return BH.find(meta) != BH.end(); // Devuelve true si la meta está en la BH
}

// Obtener el FC de la meta en la BH (NO_DEMOSTRADO si no está)
double fcEnBH(const string& meta) 
{
    auto it = BH.find(meta); // Buscar la meta en la BH
    if (it == BH.end()) 
    {
        return NO_DEMOSTRADO; // Si no está, devolver NO_DEMOSTRADO
    }
    return it->second; // Devolver el FC asociado a la meta
}

// Obtener los números de las reglas cuyo consecuente coincide con la meta
vector<int> reglasMeta(const string& meta) 
{
    vector<int> indices; // Vector para almacenar los índices de las reglas aplicables
    for (int i = 0; i < (int)BC.size(); ++i) 
    {
        if (BC[i].consecuente == meta) 
        {
            indices.push_back(i); // Añadir el índice de la regla si el consecuente coincide con la meta
        }
    }
    return indices; // Devolver el vector de índices
}

// Caso 1: combinación de antecedentes
double combinarAntecedentesCaso1(const vector<double>& listaFCAntecedentes, const string& operador) 
{
    if (listaFCAntecedentes.empty())
    { 
        return NO_DEMOSTRADO; // Si no hay antecedentes, devolver NO_DEMOSTRADO
    }
    if (listaFCAntecedentes.size() == 1) 
    {
        return listaFCAntecedentes[0]; // Si solo hay un antecedente, devolver su FC directamente
    }

    if (operador == "AND") 
    {
        double minimo = listaFCAntecedentes[0];
        for (size_t i = 1; i < listaFCAntecedentes.size(); ++i)
        {
            minimo = min(minimo, listaFCAntecedentes[i]); // AND corresponde al mínimo
        }
        return minimo;
    } 
    else if (operador == "OR") 
    {
        double maximo = listaFCAntecedentes[0];
        for (size_t i = 1; i < listaFCAntecedentes.size(); ++i)
        {
            maximo = max(maximo, listaFCAntecedentes[i]); // OR corresponde al máximo
        }
        return maximo;
    }
    // Si no hay operador válido, devolver el primer FC
    return listaFCAntecedentes[0];
}

// Caso 3: propagación por la regla
double combinarCaso3(double fcAntecedentes, double fcRegla) 
{
    return fcRegla * max(0.0, fcAntecedentes);
}

// Caso 2: combinación de dos FC
double combinarDosFCCaso2(double fc1, double fc2) 
{
    if (fc1 >= 0 && fc2 >= 0) 
    {
        return fc1 + fc2 * (1.0 - fc1); // Ambos positivos
    } 
    else if (fc1 <= 0 && fc2 <= 0) 
    {
        return fc1 + fc2 * (1.0 + fc1); // Ambos negativos
    } 
    else 
    {
        double minimoAbs = min(fabs(fc1), fabs(fc2)); // Mínimo de los valores absolutos
        double denominador = 1.0 - minimoAbs; // Denominador para normalizar
        if (denominador == 0.0) 
        {
            return 0.0; // Evitar división por cero
        }
        return (fc1 + fc2) / denominador; // Combinar cuando son de signos opuestos
    }
}

// Caso 2: combinación de una lista de FC
double combinarListaFCCaso2(const vector<double>& listaFC, const string& meta) 
{
    if (listaFC.empty())
    {
        return NO_DEMOSTRADO; // Si la lista está vacía, devolver NO_DEMOSTRADO
    }
    double fcAcumulado = listaFC[0];
    for (size_t i = 1; i < listaFC.size(); ++i) 
    {
        double nuevo = combinarDosFCCaso2(fcAcumulado, listaFC[i]); // Combinar el FC acumulado
        fcAcumulado = nuevo;
        if (ficheroLog.is_open()) 
        {
            ficheroLog << "Caso 2: " << meta << ", FC=" << fcAcumulado << "\n"; // Registrar el paso en el log
        }
    }
    return fcAcumulado; // Devolver el FC combinado total
}

// VERIFICAR(meta, BH)
double verificar(const string& meta) 
{
    // Si la meta ya está en la BH, devolver su FC
    if (contenidaEnBH(meta)) 
    {
        double fc = fcEnBH(meta); // Obtener el FC de la BH
        if (ficheroLog.is_open()) 
        {
            ficheroLog << "Hecho en BH: " << meta << ", FC=" << fc << "\n"; // Registrar en el log
        }
        return fc; // Devolver el FC encontrado en la BH
    }

    // Reglas cuyo consecuente coincide con la meta
    vector<int> indicesReglas = reglasMeta(meta); // Obtener índices de reglas que producen la meta
    vector<double> listaFC; // Contribuciones de cada regla para la meta

    for (int indice : indicesReglas) 
    {
        const Regla& regla = BC[indice]; // Referencia a la regla actual
        bool verificado = true; // Indica si todos los antecedentes se han podido demostrar
        vector<double> listaFCAntecedentes; // lista de FC de los antecedentes

        // Verificar cada antecedente recursivamente
        for (const string& nuevaMeta : regla.antecedentes) 
        {
            double fcNuevaMeta = verificar(nuevaMeta);
            if (fcNuevaMeta == NO_DEMOSTRADO) 
            {
                verificado = false; // La regla no se puede aplicar
                break;
            } 
            else 
            {
                listaFCAntecedentes.push_back(fcNuevaMeta); // Añadir FC del antecedente a la lista
            }
        }

        // Si la regla es aplicable, combinar la evidencia
        if (verificado) 
        {
            if (ficheroLog.is_open()) 
            {
                ficheroLog << regla.numero << " (regla activada)\n"; // Registrar activación de la regla en el log
            }

            // Caso 1: combinar los antecedentes (si hay más de uno)
            double fcAntecedentes = NO_DEMOSTRADO; // Valor por defecto
            if (regla.antecedentes.size() > 1) 
            {
                if (ficheroLog.is_open()) 
                {
                    for (size_t i = 0; i < regla.antecedentes.size(); ++i) 
                    {
                        ficheroLog << "  Caso 1: " << regla.antecedentes[i] << ", FC=" << listaFCAntecedentes[i] << "\n"; // Registrar cada antecedente en el log
                    }
                }
                fcAntecedentes = combinarAntecedentesCaso1(listaFCAntecedentes, regla.operador); // Combinar según el operador
                if (ficheroLog.is_open()) 
                {
                    ficheroLog << "  Caso 1 (resultado): FC_antecedentes=" << fcAntecedentes << "\n"; // Registrar resultado de la combinación en el log
                }
            } 
            else 
            {
                // Un solo antecedente: no hay combinación, se usa su FC directamente
                fcAntecedentes = listaFCAntecedentes[0]; // Tomar el FC del único antecedente
            }

            // Caso 3: propagar por la regla
            double fcConsecuente = combinarCaso3(fcAntecedentes, regla.fc);
            if (ficheroLog.is_open()) 
            {
                ficheroLog << "  Caso 3: " << regla.consecuente << ", FC=" << fcConsecuente << "\n"; // Registrar resultado del caso 3 en el log
            }
            // Añadir contribución a la lista
            listaFC.push_back(fcConsecuente);
        }
    }

    // Si ninguna regla aporta evidencia sobre la meta devolver NO_DEMOSTRADO
    if (listaFC.empty()) 
    {
        return NO_DEMOSTRADO;
    }

    // Caso 2: combinar todas las contribuciones de las reglas para obtener el FC final
    double fcTotal = combinarListaFCCaso2(listaFC, meta);

    // Guardar meta en BH con su FC
    BH[meta] = fcTotal;

    return fcTotal; // Devolver el FC final para la meta
}

// Encadenamiento hacia atrás
double encadenamientoHaciaAtras(const string& meta) 
{
    double fcMeta = verificar(meta); // Verificar la meta
    if (fcMeta != NO_DEMOSTRADO) 
    {
        return fcMeta; // Devolver el FC si se ha demostrado
    }
    return NO_DEMOSTRADO; // Devolver NO_DEMOSTRADO si no se ha podido demostrar
}

int main(int numeroArgumentos, char* argumentos[]) 
{
    if (numeroArgumentos != 3) 
    {
        cerr << "Uso: " << argumentos[0] << " <ficheroBC> <ficheroBH>\n";
        return 1; // Terminar si los argumentos no son correctos
    }
    string ficheroBC = argumentos[1]; // Primero argumento: fichero BC
    string ficheroBH = argumentos[2]; // Segundo argumento: fichero BH

    string objetivo; // Meta objetivo a demostrar

    procesarBC(ficheroBC); // Cargar la BC desde el fichero
    procesarBH(ficheroBH, objetivo); // Cargar la BH desde el fichero y obtener el objetivo

    string baseBC = ficheroBC.substr(0, ficheroBC.find_last_of("."));  // Elimina la extensión .txt de BC
    string baseBH = ficheroBH.substr(0, ficheroBH.find_last_of(".")); // Elimina la extensión .txt de BH
    string nombreSalida = "logEjecucion_" + baseBC + "_" + baseBH + ".txt"; // Nombre del fichero del log

    ficheroLog.open(nombreSalida); // Abrir el fichero de log
    if (!ficheroLog.is_open()) 
    {
        cerr << "No se pudo crear el fichero de salida: " << nombreSalida << "\n";
        return 1; // Terminar si no se puede abrir el fichero de log
    }
    ficheroLog << fixed << setprecision(5); // Formateo de salida con 5 decimales

    ficheroLog << "BC utilizada: " << ficheroBC << "\n"; // Indicar fichero BC usado en el log
    ficheroLog << "BH utilizada: " << ficheroBH << "\n"; // Indicar fichero BH usado en el log
    ficheroLog << "Objetivo: " << objetivo << "\n\n"; // Indicar objetivo en el log

    double resultado = encadenamientoHaciaAtras(objetivo); // Ejecutar el motor y obtener el resultado

    ficheroLog << "\n"; // Línea en blanco antes del resultado final
    if (resultado == NO_DEMOSTRADO) 
    {
        ficheroLog << "Objetivo " << objetivo << ", NO_DEMOSTRADO\n"; // Registrar en el log que no se ha demostrado
        cout << "Objetivo " << objetivo << ": NO_DEMOSTRADO\n";
    } 
    else 
    {
        ficheroLog << "Objetivo " << objetivo << ", FC=" << resultado << "\n"; // Registrar en el log el FC del objetivo
        cout << "FC(" << objetivo << ") = " << resultado << "\n";
    }

    ficheroLog.close(); // Cerrar el fichero de log
    return 0;
}
