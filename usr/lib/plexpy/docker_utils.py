import docker
import json
import subprocess
from pathlib import Path

class PlexDocker:
    def __init__(self):
        self.client = docker.from_env()
        self.config_file = Path("/etc/default/openmediavault-plex")
        self.config = self._load_config()
    
    def _load_config(self):
        config = {}
        if self.config_file.exists():
            with open(self.config_file, 'r') as f:
                for line in f:
                    if line.strip() and not line.startswith('#'):
                        key, value = line.split('=', 1)
                        config[key.strip()] = value.strip().strip('"')
        return config
    
    def is_plex_in_docker(self):
        try:
            containers = self.client.containers.list(all=True)
            for container in containers:
                if self.config.get('PLEX_DOCKER_NAME', 'plex') in container.name:
                    return True
            return False
        except:
            return False
    
    def get_docker_plex_info(self):
        try:
            container = self.client.containers.get(self.config.get('PLEX_DOCKER_NAME', 'plex'))
            return {
                'status': container.status,
                'image': container.image.tags,
                'ports': container.ports,
                'volumes': container.attrs['Mounts'],
                'created': container.attrs['Created'],
                'running': container.status == 'running'
            }
        except:
            return None
    
    def control_docker_plex(self, action):
        try:
            container = self.client.containers.get(self.config.get('PLEX_DOCKER_NAME', 'plex'))
            
            if action == 'start':
                container.start()
            elif action == 'stop':
                container.stop()
            elif action == 'restart':
                container.restart()
            elif action == 'update':
                self.update_docker_plex()
            
            return True
        except Exception as e:
            print(f"Error controlling docker: {str(e)}")
            return False
    
    def update_docker_plex(self):
        try:
            subprocess.run([
                "docker", "pull", 
                f"{self.config.get('PLEX_DOCKER_IMAGE', 'plexinc/pms-docker')}:"
                f"{self.config.get('PLEX_DOCKER_TAG', 'latest')}"
            ], check=True)
            
            container = self.client.containers.get(self.config.get('PLEX_DOCKER_NAME', 'plex'))
            container.stop()
            container.remove()
            
            # Recreate container with same parameters
            volumes = {
                '/path/to/config': {'bind': '/config', 'mode': 'rw'},
                '/path/to/media': {'bind': '/data', 'mode': 'ro'}
            }
            
            self.client.containers.run(
                image=f"{self.config.get('PLEX_DOCKER_IMAGE', 'plexinc/pms-docker')}:"
                      f"{self.config.get('PLEX_DOCKER_TAG', 'latest')}",
                name=self.config.get('PLEX_DOCKER_NAME', 'plex'),
                network_mode=self.config.get('PLEX_DOCKER_NETWORK', 'host'),
                volumes=volumes,
                restart_policy={"Name": "unless-stopped"},
                detach=True
            )
            
            return True
        except Exception as e:
            print(f"Error updating docker: {str(e)}")
            return False
