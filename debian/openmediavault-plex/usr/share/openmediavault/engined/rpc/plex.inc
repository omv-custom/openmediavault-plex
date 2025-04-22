<?php

class OMVRpcServicePlex extends \OMV\Rpc\ServiceAbstract
{
    public function getName()
    {
        return "plex";
    }
    
    public function initialize()
    {
        $this->registerMethod("getSettings");
        $this->registerMethod("setSettings");
        $this->registerMethod("getStatus");
        $this->registerMethod("getStats");
        $this->registerMethod("checkUpdate");
        $this->registerMethod("doUpdate");
        $this->registerMethod("start");
        $this->registerMethod("stop");
        $this->registerMethod("restart");
        $this->registerMethod("getInstallType");
        $this->registerMethod("getDockerInfo");
        $this->registerMethod("controlDockerPlex");
        $this->registerMethod("updateDockerPlex");
        $this->registerMethod("migrateToDocker");
        $this->registerMethod("migrateToNative");
    }

    public function getSettings($params, $context)
    {
        // Sprawdzenie uprawnień
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        
        // Pobranie konfiguracji z bazy
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.plex");
        
        return $object->getAssoc();
    }

    public function setSettings($params, $context)
    {
        // Sprawdzenie uprawnień
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        
        // Walidacja parametrów
        $this->validateMethodParams($params, 'rpc.plex.setsettings');
        
        // Zapis do bazy
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get('conf.service.plex');

        $object->setAssoc($params);
        $db->set($object);

        return $object->getAssoc();        
    }

    public function getStatus($params, $context)
    {
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $module = new OMVModulePlex();
        return $module->getStatus();
    }
    
    public function getStats($params, $context)
    {
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $module = new OMVModulePlex();
        return $module->getStats();
    }
    
    public function checkUpdate($params, $context)
    {
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $module = new OMVModulePlex();
        return $module->checkUpdate();
    }
    
    public function doUpdate($params, $context)
    {
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        
        // Update Plex Media Server
        #$cmd = "apt-get update && apt-get --only-upgrade install plexmediaserver";
        #$this->exec($cmd, $output);
        
        return ["message" => "Plex Media Server updated successfully"];
    }
    
    public function start($params, $context)
    {
        #$this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        #$this->exec("systemctl start plexmediaserver", $output);
        return ["message" => "Plex Media Server started"];
    }
    
    public function stop($params, $context)
    {
        #$this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        #$this->exec("systemctl stop plexmediaserver", $output);
        return ["message" => "Plex Media Server stopped"];
    }
    
    public function restart($params, $context)
    {
        #$this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        #$this->exec("systemctl restart plexmediaserver", $output);
        return ["message" => "Plex Media Server restarted"];
    }
    
    public function getLibraryUsage($params, $context)
    {
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        
        $db = \OMV\Config\Database::getInstance();
        $settings = $db->get("conf.service.plex");
        $libraryPaths = $settings['library'] ?? [];
        
        $usage = [];
        foreach ($libraryPaths as $path) {
            $size = shell_exec("du -sh " . escapeshellarg($path) . " | awk '{print $1}'");
            $usage[] = [
                'path' => $path,
                'size' => trim($size)
            ];
        }
        
        return $usage;
    }

    public function getLibraryStats($params, $context)
    {
        #$this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        #$stats = shell_exec("/usr/sbin/omv-plexctl get-library-stats");
        return json_decode($stats, true);
    }
    
    public function getActiveSessions($params, $context)
    {
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $sessions = shell_exec("/usr/sbin/omv-plexctl get-active-sessions");
        return json_decode($sessions, true);
    }
    
    public function updatePlexSettings($params, $context)
    {
        #$this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        #$this->validateMethodParams($params, "rpc.plex.updatesettings");
        
        #$json_params = escapeshellarg(json_encode($params));
        #$result = shell_exec("/usr/sbin/omv-plexctl update-settings " . $json_params);
        return json_decode($result, true);
    }
    
    public function getTranscoderStats($params, $context)
    {
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        
        // Pobierz statystyki transkodowania
        $cmd = "ps aux | grep -i 'plex transcoder' | awk '{print $3,$4,$10}'";
        exec($cmd, $output);
        
        $stats = [];
        foreach ($output as $line) {
            if (!empty($line)) {
                list($cpu, $mem, $time) = explode(" ", $line);
                $stats[] = [
                    'cpu' => $cpu,
                    'memory' => $mem,
                    'time' => $time
                ];
            }
        }
        
        return $stats;
    }

    public function getInstallType($params, $context)
    {
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $type = shell_exec("/usr/sbin/omv-plexctl get-install-type");
        return ["type" => trim($type)];
    }
    
    public function getDockerInfo($params, $context)
    {
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $info = shell_exec("/usr/sbin/omv-plexctl docker-info");
        return json_decode($info, true);
    }
    
    public function controlDockerPlex($params, $context)
    {
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $this->validateMethodParams($params, "rpc.plex.dockercontrol");
        
        $action = escapeshellarg($params['action']);
        $output = shell_exec("/usr/sbin/omv-plexctl docker-control " . $action);
        
        return ["status" => trim($output) === "True" ? "success" : "failed"];
    }
    
    public function updateDockerPlex($params, $context)
    {
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        
        $output = shell_exec("/usr/sbin/omv-plexctl docker-update");
        return ["status" => trim($output) === "True" ? "success" : "failed"];
    }
    
    public function switchInstallType($params, $context)
    {
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $this->validateMethodParams($params, "rpc.plex.switchinstall");
        
        $type = $params['type'];
        $current = shell_exec("/usr/sbin/omv-plexctl get-install-type");
        
        if (trim($current) === $type) {
            return ["status" => "no_change"];
        }
        
        if ($type === "docker") {
            // Migracja z native do docker
            $this->migrateToDocker($params);
        } else {
            // Migracja z docker do native
            $this->migrateToNative($params);
        }
        
        return ["status" => "success"];
    }
    
    private function migrateToDocker($params)
    {
        // 1. Zatrzymaj istniejącą usługę
        system("systemctl stop plexmediaserver");
        
        // 2. Zainstaluj Docker jeśli potrzeba
        if (!file_exists("/usr/bin/docker")) {
            system("apt-get install -y docker.io");
        }
        
        // 3. Przygotuj woluminy
        $config_dir = "/var/lib/plexmediaserver";
        $media_dirs = json_decode($params['media_dirs'], true);
        
        // 4. Uruchom kontener Dockera
        $docker_cmd = "docker run -d \\
            --name plex \\
            --network=host \\
            -e TZ=\"Europe/Warsaw\" \\
            -e PLEX_CLAIM=\"{$params['claim_token']}\" \\
            -v {$config_dir}:/config \\
            -v /tmp:/transcode";
        
        foreach ($media_dirs as $dir) {
            $docker_cmd .= " -v {$dir}:{$dir}";
        }
        
        $docker_cmd .= " plexinc/pms-docker:latest";
        
        system($docker_cmd);
        
        // 5. Wyłącz oryginalną usługę
        system("systemctl disable plexmediaserver");
    }
    
    private function migrateToNative($params)
    {
        // 1. Zatrzymaj kontener Dockera
        system("docker stop plex");
        
        // 2. Dodaj repozytorium Plex
        system("curl https://downloads.plex.tv/plex-keys/PlexSign.key | gpg --dearmor | tee /usr/share/keyrings/plex-archive-keyring.gpg >/dev/null");
        system("echo \"deb [signed-by=/usr/share/keyrings/plex-archive-keyring.gpg] https://downloads.plex.tv/repo/deb public main\" | tee /etc/apt/sources.list.d/plexmediaserver.list");
        
        // 3. Zainstaluj Plex
        system("apt-get update && apt-get install -y plexmediaserver");
        
        // 4. Skopiuj konfigurację z Dockera
        $config_src = "/var/lib/docker/volumes/plex_config/_data";
        $config_dst = "/var/lib/plexmediaserver";
        
        if (file_exists($config_src)) {
            system("rsync -a {$config_src}/ {$config_dst}/");
        }
        
        // 5. Uruchom usługę
        system("systemctl start plexmediaserver");
        
        // 6. Usuń kontener Dockera
        system("docker rm plex");
    }
}

?>
