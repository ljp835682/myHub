using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class NpcMainManager : MonoBehaviour {

    private Animator ani;

	// Use this for initialization
	void Start () {
        ani = GetComponent<Animator>();
    }
	
	// Update is called once per frame
	void Update () {
    }

    public void SpecialAni()
    {
        ani.SetBool("isBuy", true);
    }

    public void BaseAni()
    {
        ani.SetBool("isBuy", false);
    }
}
