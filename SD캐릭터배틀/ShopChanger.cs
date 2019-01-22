using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class ShopChanger : MonoBehaviour {

    //private int state = 0;
    public GameObject field;
    public GameObject charashop;
    public GameObject Medicineshop;
    public GameObject box;
    private AudioSource audio;
    public AudioClip trueClip;

    // Use this for initialization
    void Start () {
        audio = GetComponent<AudioSource>();
        Box();
    }
	
	// Update is called once per frame
	void Update () {
		
	}

    public void Back()
    {
        SceneManager.LoadScene("Game");
    }

    public void Box()
    {
        charashop.SetActive(false);
        Medicineshop.SetActive(false);
        box.SetActive(true);

        if (transform.childCount != 0)
        {
            Destroy(transform.GetChild(0).gameObject);
        }

        field.GetComponent<SpriteRenderer>().sprite = Resources.Load<Sprite>("Image/Shop_B");
        Vector2 vetor = new Vector2(transform.position.x, transform.position.y);
        GameObject obj = Instantiate(Resources.Load("Prefabs/NPC/NPC_02"), vetor, Quaternion.identity) as GameObject;
        obj.transform.parent = gameObject.transform;
    }

    public void Chara()
    {
        charashop.SetActive(true);
        Medicineshop.SetActive(false);
        box.SetActive(false);

        if(transform.childCount != 0)
        {
            Destroy(transform.GetChild(0).gameObject);
        }
        field.GetComponent<SpriteRenderer>().sprite = Resources.Load<Sprite>("Image/Shop_C");
        Vector2 vetor = new Vector2(transform.position.x, transform.position.y);
        GameObject obj = Instantiate(Resources.Load("Prefabs/NPC/NPC_00"), vetor, Quaternion.identity) as GameObject;
        obj.transform.parent = gameObject.transform;
    }

    public void Medicine()
    {
        charashop.SetActive(false);
        Medicineshop.SetActive(true);
        box.SetActive(false);
        if (transform.childCount != 0)
        {
            Destroy(transform.GetChild(0).gameObject);
        }
        field.GetComponent<SpriteRenderer>().sprite = Resources.Load<Sprite>("Image/Shop_M");
        Vector2 vetor = new Vector2(transform.position.x, transform.position.y);
        GameObject obj = Instantiate(Resources.Load("Prefabs/NPC/NPC_01"), vetor, Quaternion.identity) as GameObject;
        obj.transform.parent = gameObject.transform;
    }

    public void TrueAudio()
    {
        audio.Stop();
        audio.clip = trueClip;
        audio.Play();
    }
}
