using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BGMPlayer : MonoBehaviour {

    private AudioSource audio;          //오디오 컴포넌트
    public AudioClip found;             //찾았을때의 음악
    public AudioClip notfound;          //찾지 못했을때의 음악

	//음악 재생 초기화
	void Start () {
        audio = GetComponent<AudioSource>();
        audio.loop = true;
        audio.volume = 0.05f;

        audio.clip = notfound;
        audio.Play();
    }

    //찾았을때
    public void FoundBGM()
    {
        audio.clip = found;
        audio.Stop();
        audio.Play();
    }

    //못찾앗을때
    public void NotFoundBGM()
    {
        audio.clip = notfound;
        audio.Stop();
        audio.Play();
    }
}
