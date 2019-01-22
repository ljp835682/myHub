using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CharaShop : MonoBehaviour {

    public Transform npcPos;
    public AudioClip trueClip;
    public AudioClip falseClip;
    private AudioSource audio;

    // Use this for initialization
    void Start () {
        audio = GetComponent<AudioSource>();
    }
	
	// Update is called once per frame
	void Update () {
		
	}

    public void Button1()
    {
        if (PlayableDatabase.Getinst.BuyChara(1, 700))
        {
            npcPos.GetChild(0).GetComponent<NpcMainManager>().SpecialAni();
            TrueAudio();
        }

        else
        {
            FalseAudio();
        }
    }

    public void Button2()
    {
        if (PlayableDatabase.Getinst.BuyChara(2, 600))
        {
            npcPos.GetChild(0).GetComponent<NpcMainManager>().SpecialAni();
            TrueAudio();
        }

        else
        {
            FalseAudio();
        }
    }

    public void Button3()
    {
        if (PlayableDatabase.Getinst.BuyChara(3, 800))
        {
            TrueAudio();
        }

        else
        {
            FalseAudio();
        }
    }

    public void Button4()
    {
        if (PlayableDatabase.Getinst.BuyChara(4, 999))
        {
            TrueAudio();
        }

        else
        {
            FalseAudio();
        }
    }

    public void Button5()
    {
        if (PlayableDatabase.Getinst.BuyChara(5, 600))
        {
            TrueAudio();
        }

        else
        {
            FalseAudio();
        }
    }

    public void TrueAudio()
    {
        audio.Stop();
        audio.clip = trueClip;
        audio.Play();
    }

    public void FalseAudio()
    {
        audio.Stop();
        audio.clip = falseClip;
        audio.Play();
    }
}
