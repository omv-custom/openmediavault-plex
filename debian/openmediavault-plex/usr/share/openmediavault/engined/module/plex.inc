<?php

class OMVModulePlex extends \OMV\Engine\Module\ServiceAbstract implements \OMV\Engine\Module\IServiceStatus
{
    public function getName()
    {
        return "plex";
    }

    public function getStatus()
    {
        try {
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.plex");
        $data = $object->getAssoc();
            
            $status = [
                "name" => $this->getName(),
                "title" => "Plex Media Server",
                "enabled" => boolval($data['enable']),
                "running" => false,
                "version" => "unknown",
                "installation" => "none",
                "settings" => $data
            ];

            // Check Docker installation
            $dockerRunning = false;
            @exec("docker inspect --format='{{.State.Running}}' plex 2>/dev/null", $output, $ret);
            if ($ret === 0 && trim(implode("", $output)) === "true") {
                $status['installation'] = "docker";
                $status['running'] = true;
                
                // Get Docker version
                @exec("docker exec plex printenv PLEX_VERSION 2>/dev/null", $versionOutput, $versionRet);
                if ($versionRet === 0 && !empty($versionOutput)) {
                    $status['version'] = trim(implode("", $versionOutput));
                }
            } 
            // Check system service only if Docker not found
            else {
                @exec("systemctl is-active plexmediaserver 2>/dev/null", $sysOutput, $sysRet);
                if ($sysRet === 0) {
                    $status['installation'] = "system";
                    $status['running'] = true;
                    
                    // Get system version
                    @exec("/usr/sbin/omv-plexctl get-version 2>/dev/null", $verOutput, $verRet);
                    if ($verRet === 0 && !empty($verOutput)) {
                        $status['version'] = trim(implode("", $verOutput));
                    }
                }
            }

            return $status;

        } catch (\Exception $e) {
            // Return minimal status info if something fails
            return [
                "name" => $this->getName(),
                "title" => "Plex Media Server",
                "enabled" => false,
                "running" => false,
                "version" => "unknown",
                "installation" => "error"
            ];
        }
    }

    public function getStats()
    {
        try {
            // Try Docker first
            @exec("docker exec plex /usr/sbin/omv-plexctl get-stats 2>/dev/null", $output, $ret);
            if ($ret !== 0 || empty($output)) {
                // Fall back to system
                @exec("/usr/sbin/omv-plexctl get-stats 2>/dev/null", $output, $ret);
            }
            
            if ($ret === 0 && !empty($output)) {
                return json_decode(implode("", $output), true) ?: [];
            }
        } catch (\Exception $e) {
            // Ignore errors
        }
        return [];
    }

    public function checkUpdate()
    {
        try {
            // Try Docker first
            @exec("docker exec plex /usr/sbin/omv-plexctl check-update 2>/dev/null", $output, $ret);
            if ($ret !== 0 || empty($output)) {
                // Fall back to system
                @exec("/usr/sbin/omv-plexctl check-update 2>/dev/null", $output, $ret);
            }
            
            if ($ret === 0 && !empty($output)) {
                return json_decode(implode("", $output), true) ?: [];
            }
        } catch (\Exception $e) {
            // Ignore errors
        }
        return [];
    }
}
?>
