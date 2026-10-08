"""Explicit, owner-supplied additions to the local reading source."""
import json
from pathlib import Path


def john_8():
    path = Path(__file__).resolve().parents[1] / 'data/source/john-8-12-20.json'
    if not path.exists():
        raise FileNotFoundError(f'Required owner-supplied Gospel supplement: {path}')
    data = json.loads(path.read_text())
    assert data['id'] == 'john-8-12-20'
    assert data['citation'] == 'John 8: 12–20'
    assert len(data['paragraphs']) == 5 and all(data['paragraphs'])
    return data
