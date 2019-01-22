using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class SceneLoader: MonoBehaviour {

    private AudioSource audio;
    public AudioClip trueClip;

    // Use this for initialization
    void Start () {
        PlayableDatabase.Create();
	}
	
	// Update is called once per frame
	void Update () {
        audio = GetComponent<AudioSource>();
    }
    public void  TitleSceneLoad()
    {
        PlayableDatabase.Getinst.Init();
        SceneManager.LoadScene("Title");
        TrueAudio();
    }

    public void ShopSceneLoad()
    {
        SceneManager.LoadScene("Shop");
        TrueAudio();
    }

    public void GameSceneLoad()
    {
        SceneManager.LoadScene("Game");
        TrueAudio();
    }

    public void ShutDown()
    {
        TrueAudio();
        Application.Quit();
    }

    public void TrueAudio()
    {
        audio.Stop();
        audio.clip = trueClip;
        audio.Play();
    }
}
