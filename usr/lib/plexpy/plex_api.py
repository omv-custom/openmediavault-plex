import requests
import json
from pathlib import Path

class PlexAPI:
    def __init__(self):
        self.token = self._get_plex_token()
        self.base_url = "http://localhost:32400"
        self.headers = {
            "X-Plex-Token": self.token,
            "Accept": "application/json"
        }
    
    def _get_plex_token(self):
        """Pobierz token Plex z preferencji"""
        prefs_path = Path("/var/lib/plexmediaserver/Library/Application Support/Plex Media Server/Preferences.xml")
        if prefs_path.exists():
            with open(prefs_path, 'r') as f:
                content = f.read()
                start = content.find('PlexOnlineToken="') + 17
                end = content.find('"', start)
                return content[start:end]
        return ""
    
    def get_library_stats(self):
        """Pobierz statystyki biblioteki"""
        url = f"{self.base_url}/library/sections"
        response = requests.get(url, headers=self.headers)
        if response.status_code == 200:
            data = response.json()
            stats = {
                'movies': 0,
                'tv_shows': 0,
                'music': 0,
                'photos': 0
            }
            
            for section in data['MediaContainer']['Directory']:
                if section['type'] == 'movie':
                    stats['movies'] = section['count']
                elif section['type'] == 'show':
                    stats['tv_shows'] = section['count']
                elif section['type'] == 'artist':
                    stats['music'] = section['count']
                elif section['type'] == 'photo':
                    stats['photos'] = section['count']
            
            return stats
        return None
    
    def get_active_sessions(self):
        """Pobierz aktywne sesje"""
        url = f"{self.base_url}/status/sessions"
        response = requests.get(url, headers=self.headers)
        if response.status_code == 200:
            data = response.json()
            sessions = []
            
            if 'MediaContainer' in data and 'Metadata' in data['MediaContainer']:
                for session in data['MediaContainer']['Metadata']:
                    sessions.append({
                        'user': session.get('User', {}).get('title', 'Unknown'),
                        'title': session.get('title', 'Unknown'),
                        'type': session.get('type', 'Unknown'),
                        'player': session.get('Player', {}).get('title', 'Unknown'),
                        'progress': f"{round(session.get('viewOffset', 0) / session.get('duration', 1) * 100, 1)}%"
                    })
            
            return sessions
        return None
    
    def update_settings(self, settings):
        """Aktualizuj ustawienia Plex"""
        url = f"{self.base_url}/:/prefs"
        params = {k: v for k, v in settings.items() if v is not None}
        response = requests.put(url, headers=self.headers, params=params)
        return response.status_code == 200
