using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CharaBox : MonoBehaviour {

    public Transform npcPos;
    public AudioClip trueClip;
    public AudioClip falseClip;
    private AudioSource audio;

    // Use this for initialization
    void Start () {
        audio = GetComponent<AudioSource>();
    }
	
	// Update is called once per frame

    public void Arm(int index)
    {
        if (PlayableDatabase.Getinst.GotoEquip(index))
        {
            npcPos.GetChild(0).GetComponent<NpcMainManager>().SpecialAni();
            TrueAudio();
        }

        else
        {
            FalseAudio();
        }
    }

    public void DisArm(int index)
    {
        if (PlayableDatabase.Getinst.GotoBox(index))
        {
            npcPos.GetChild(0).GetComponent<NpcMainManager>().SpecialAni();
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
